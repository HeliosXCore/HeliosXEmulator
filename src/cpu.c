#include "cpu.h"
#include "decode.h"
#include <stdbool.h>
#include "difftest.h"
#include "memory.h"
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include "common.h"

extern CPU_state cpu;
extern NEMUState nemu_state;

// static const uint32_t img[] = {
//     0x00000297,  // auipc t0,0
//     0x00028823,  // sb  zero,16(t0)
//     0x0102c503,  // lbu a0,16(t0)
//     0x00100073,  // ebreak (used as nemu_trap)
//     0xdeadbeef,  // some data
// };

__attribute__((noinline)) void invalid_inst(uint32_t thispc) {
    uint32_t temp[2];
    uint32_t pc = thispc;
    temp[0] = inst_fetch(&pc, 4);
    temp[1] = inst_fetch(&pc, 4);

    uint8_t *p = (uint8_t *)temp;
    printf("invalid opcode(PC = " FMT_WORD
           "):\n"
           "\t%02x %02x %02x %02x %02x %02x %02x %02x ...\n"
           "\t%08x %08x...\n",
           thispc, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], temp[0],
           temp[1]);

    printf(
        "There are two cases which will trigger this unexpected exception:\n"
        "1. The instruction at PC = " FMT_WORD
        " is not implemented.\n"
        "2. Something is implemented incorrectly.\n",
        thispc);
    printf("Find this PC(" FMT_WORD
           ") in the disassembling result to distinguish which case it is.\n\n",
           thispc);
    printf(ANSI_FMT(
        "If it is the first case, see\nriscv manual\nfor more details.\n\n"
        "If it is the second case, remember:\n"
        "* The machine is always right!\n"
        "* Every line of untested code is always wrong!\n\n",
        ANSI_FG_RED));

    set_nemu_state(NEMU_ABORT, thispc, -1);
}

void set_nemu_state(int state, uint32_t pc, int halt_ret) {
    nemu_state.state = state;
    nemu_state.halt_pc = pc;
    nemu_state.halt_ret = halt_ret;
}

static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2,
                           word_t *imm, int type) {
    uint32_t i = s->isa.inst.val;
    int rs1 = BITS(i, 19, 15);
    int rs2 = BITS(i, 24, 20);
    *rd = BITS(i, 11, 7);
    switch (type) {
        case TYPE_J:
            immJ();
            break;
        case TYPE_I:
            src1R();
            immI();
            break;
        case TYPE_U:
            immU();
            break;
        case TYPE_R:
            src1R();
            src2R();
            break;
        case TYPE_S:
            src1R();
            src2R();
            immS();
            break;
        case TYPE_B:
            src1R();
            src2R();
            immB();
            break;
    }
}

static int decode_exec(Decode *s, DifftestResult *difftest_result) {
    difftest_result->pc = s->pc;
    int rd = 0;
    uint32_t src1 = 0, src2 = 0, imm = 0;
    s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */)             \
    {                                                                    \
        decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
        __VA_ARGS__;                                                     \
        switch (concat(TYPE_, type)) {                                   \
            case TYPE_R:                                                 \
            case TYPE_I:                                                 \
            case TYPE_U:                                                 \
            case TYPE_J:                                                 \
                difftest_result->reg_id = rd;                            \
                difftest_result->wen = 1;                                \
                difftest_result->reg_val = cpu.gpr[rd];                  \
                break;                                                   \
            default:                                                     \
                difftest_result->wen = 0;                                \
        }                                                                \
    }

    INSTPAT_START();

    INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu, B,
            if (src1 >= src2) s->dnpc = s->pc + imm);

    INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu, I,
            R(rd) = Mr(src1 + imm, 2));

    INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh, I,
            R(rd) = (word_t)((sword_t)(int16_t)((uint16_t)Mr(src1 + imm, 2))));

    INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh, S,
            Mw(src1 + imm, 2, src2));

    INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra, R,
            R(rd) = (word_t)((sword_t)src1 >> (src2 & 0x0000001f)));

    INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu, R,
            R(rd) = src1 / src2);

    INSTPAT("0000001 ????? ????? 111 ????? 01100 11", remu, R,
            R(rd) = src1 % src2);

    INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt, B,
            if ((sword_t)src1 < (sword_t)src2) s->dnpc = s->pc + imm);

    INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll, R,
            R(rd) = src1 << (src2 & 0x0000001f));

    INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu, B,
            if (src1 < src2) s->dnpc = s->pc + imm);

    INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori, I,
            R(rd) = src1 ^ imm);

    INSTPAT("0000000 ????? ????? 101 ????? 00100 11", srli, I,
            R(rd) = src1 >> (imm & 0x0000001f));

    INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl, R,
            R(rd) = src1 >> (src2 & 0x0000001f));

    INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and, R,
            R(rd) = src1 & src2);

    INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu, R,
            R(rd) = src1 < src2);

    INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor, R,
            R(rd) = src1 ^ src2);

    INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt, R,
            R(rd) = (sword_t)src1 < (sword_t)src2);

    INSTPAT("0100000 ????? ????? 101 ????? 00100 11", srai, I,
            R(rd) = (word_t)((sword_t)src1 >> (imm & 0x0000001f)));

    INSTPAT(
        "0000001 ????? ????? 001 ????? 01100 11", mulh, R,
        R(rd) =
            (word_t)(((int64_t)(sword_t)src1 * (int64_t)(sword_t)src2) >> 32));

    INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi, I,
            R(rd) = (src1 & imm));

    INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge, B,
            if ((sword_t)src1 >= (sword_t)src2) s->dnpc = s->pc + imm);

    INSTPAT("0000001 ????? ????? 100 ????? 01100 11", div, R,
            R(rd) = (sword_t)src1 / (sword_t)src2);

    INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul, R,
            R(rd) = (sword_t)src1 * (sword_t)src2);

    INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or, R,
            R(rd) = src1 | src2);

    INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub, R,
            R(rd) = src1 - src2);

    INSTPAT("0000000 ????? ????? 001 ????? 00100 11", slli, I,
            R(rd) = (src1 << (imm & 0x0000001f)));

    INSTPAT("0000001 ????? ????? 110 ????? 01100 11", rem, R,
            // R(rd) = src1 % src2);
            R(rd) = ((sword_t)src1 >= 0 ? (src1 % src2)
                                        : (word_t)(-(sword_t)(src1 % src2))));

    INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu, I,
            R(rd) = (src1 < imm));

    INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add, R,
            R(rd) = src1 + src2);

    INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw, I,
            R(rd) = Mr(src1 + imm, 4));

    INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui, U, R(rd) = imm);

    INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne, B,
            if (src1 != src2) s->dnpc = s->pc + imm);

    INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq, B,
            if (src1 == src2) s->dnpc = s->pc + imm);

    // 00008067          	ret
    INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr, I,
            s->dnpc = (src1 + imm) & 0xfffffffe;
            R(rd) = s->pc + 4;);

    // 00112023          	sw	ra,0(sp)
    INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw, S,
            Mw(src1 + imm, 4, src2));

    // 00c000ef
    INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal, J,
            s->dnpc = s->pc + imm;
            R(rd) = s->pc + 4;);

    // 00000413: 0 0 0 0 0  000  0100  0    001 0011
    // 0010011 (19) 000 – I addi rd, rs1, imm
    INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi, I,
            R(rd) = src1 + imm);

    INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc, U,
            R(rd) = s->pc + imm);
    INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu, I,
            R(rd) = Mr(src1 + imm, 1));
    INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb, S,
            Mw(src1 + imm, 1, src2));

    INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak, N,
            NEMUTRAP(s->pc, R(10)));  // R(10) is $a0
    INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv, N, INV(s->pc));
    INSTPAT_END();

    R(0) = 0;  // reset $zero to 0

    return 0;
}

static int isa_exec_once(Decode *s, DifftestResult *difftest_result) {
    s->isa.inst.val = inst_fetch(&s->snpc, 4);
    return decode_exec(s, difftest_result);
}

static void exec_once(Decode *s, uint32_t pc, DifftestResult *difftest_result) {
    s->pc = pc;
    s->snpc = pc;
    isa_exec_once(s, difftest_result);
    cpu.pc = s->dnpc;
}

static void execute(uint64_t n, DifftestResult *difftest_result) {
    // TODO
    Decode s;
    for (; n > 0; n--) {
        exec_once(&s, cpu.pc, difftest_result);
        // trace_and_difftest(&s, cpu.pc);
        if (nemu_state.state != NEMU_RUNNING) break;
    }
}

void cpu_exec(uint64_t n, DifftestResult *difftest_result) {
    switch (nemu_state.state) {
        case NEMU_END:
        case NEMU_ABORT:
            printf(
                "Program execution has ended. To restart the program, exit "
                "NEMU and "
                "run again.\n");
            return;
        default:
            nemu_state.state = NEMU_RUNNING;
    }
    execute(n, difftest_result);
    switch (nemu_state.state) {
        case NEMU_RUNNING:
            nemu_state.state = NEMU_STOP;
            break;

        case NEMU_END:
        case NEMU_ABORT:
            Log("nemu: %s at pc = " FMT_WORD,
                (nemu_state.state == NEMU_ABORT
                     ? ANSI_FMT("ABORT", ANSI_FG_RED)
                     : (nemu_state.halt_ret == 0
                            ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN)
                            : ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
                nemu_state.halt_pc);
        case NEMU_QUIT:
            Log("nemu quited\n");
    }
}

static void restart() {
    /* Set the initial program counter. */
    cpu.pc = RESET_VECTOR;

    /* The zero register is always 0. */
    cpu.gpr[0] = 0;
}

void init_isa() {
    /* Load built-in image. */
    // memcpy(guest_to_host(RESET_VECTOR), img, sizeof(img));

    /* Initialize this virtual computer system. */
    restart();

    nemu_state.state = NEMU_STOP;
}
