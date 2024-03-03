#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include "cpu.h"
#include "difftest.h"
#include "memory.h"
#include <string.h>

extern CPU_state cpu;
extern NEMUState nemu_state;

const char *regs[32] = {"$0", "ra", "sp",  "gp",  "tp", "t0", "t1", "t2",
                        "s0", "s1", "a0",  "a1",  "a2", "a3", "a4", "a5",
                        "a6", "a7", "s2",  "s3",  "s4", "s5", "s6", "s7",
                        "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

const char *reg_idx2str(const uint32_t idx) { return regs[idx]; }

uint32_t get_regidx(const char *regname) {
    for (int i = 0; i < 32; i++) {
        if (strcmp(regname, regs[i]) == 0) {
            return i;
        }
    }
    printf("regname is not correct!\n");
    return -1;
}

int main(int argc, char *argv[]) {
    difftest_init();

    const uint32_t img[] = {

        0x00000297,  // auipc t0,0
        0x00028823,  // sb  zero,16(t0)
        0x0102c503,  // lbu a0,16(t0)
        0x00100073,  // ebreak (used as nemu_trap)
        0xdeadbeef,  // some data
    };

    const uint32_t img1[] = {
        0x00000413,  // li s0, 0         -> addi s0, x0, 0
        0x74300613,  // li a2, 1859      -> addi a2, x0, 1859
        0x00860433,  // add s0, a2, s0
        0x3a100713,  // li a4, 929       -> addi a4, x0, 929
        0x01600793,  // li a5, 22        -> addi a5, x0, 22
        0x00f70533,  // add a0, a4, a5
        0x00100073,  // ebreak (used as nemu_trap)
    };

    // difftest_memcpy_from_dut(RESET_VECTOR, (uint32_t *)img, sizeof(img));
    difftest_memcpy_from_dut(RESET_VECTOR, (uint32_t *)img1, sizeof(img1));
    DifftestResult diffresult;

    // test inst1
    difftest_exec(1, &diffresult);
    assert(diffresult.wen == 1);
    assert(diffresult.pc == 0x80000000);
    assert(diffresult.reg_id == get_regidx("s0"));
    assert(diffresult.reg_val == 0);
    CPU_state r;
    difftest_getregs((void *)&r);
    for (int i = 0; i < 32; i++) {
        assert(r.gpr[i] == cpu.gpr[i]);
    }
    assert(r.pc == cpu.pc);
    printf("%s\n", "inst1 test pass!");

    // test inst2
    difftest_exec(1, &diffresult);
    assert(diffresult.wen == 1);
    assert(diffresult.pc == 0x80000004);
    assert(diffresult.reg_id == get_regidx("a2"));
    assert(diffresult.reg_val == 1859);
    difftest_getregs((void *)&r);
    for (int i = 0; i < 32; i++) {
        assert(r.gpr[i] == cpu.gpr[i]);
    }
    assert(r.pc == cpu.pc);
    printf("%s\n", "inst2 test pass!");

    // test inst3
    difftest_exec(1, &diffresult);
    assert(diffresult.wen == 1);
    assert(diffresult.pc == 0x80000008);
    assert(diffresult.reg_id == get_regidx("s0"));
    assert(diffresult.reg_val == 1859);
    difftest_getregs((void *)&r);
    for (int i = 0; i < 32; i++) {
        assert(r.gpr[i] == cpu.gpr[i]);
    }
    assert(r.pc == cpu.pc);
    printf("%s\n", "inst3 test pass!");

    // test inst4
    difftest_exec(1, &diffresult);
    assert(diffresult.wen == 1);
    assert(diffresult.pc == 0x8000000c);
    assert(diffresult.reg_id == get_regidx("a4"));
    assert(diffresult.reg_val == 929);
    difftest_getregs((void *)&r);
    for (int i = 0; i < 32; i++) {
        assert(r.gpr[i] == cpu.gpr[i]);
    }
    assert(r.pc == cpu.pc);
    printf("%s\n", "inst4 test pass!");

    // test inst5
    difftest_exec(1, &diffresult);
    assert(diffresult.wen == 1);
    assert(diffresult.pc == 0x80000010);
    assert(diffresult.reg_id == get_regidx("a5"));
    assert(diffresult.reg_val == 22);
    difftest_getregs((void *)&r);
    for (int i = 0; i < 32; i++) {
        assert(r.gpr[i] == cpu.gpr[i]);
    }
    assert(r.pc == cpu.pc);
    printf("%s\n", "inst5 test pass!");

    // test inst6
    difftest_exec(1, &diffresult);
    assert(diffresult.wen == 1);
    assert(diffresult.pc == 0x80000014);
    assert(diffresult.reg_id == get_regidx("a0"));
    assert(diffresult.reg_val == 951);
    difftest_getregs((void *)&r);
    for (int i = 0; i < 32; i++) {
        assert(r.gpr[i] == cpu.gpr[i]);
    }
    assert(r.pc == cpu.pc);
    printf("%s\n", "inst6 test pass!");

    r.pc = RESET_VECTOR;
    difftest_setregs((void *)&r);
    printf("%s:\t%x\n", reg_idx2str(10), r.gpr[10]);
    difftest_exec(-1, &diffresult);
    difftest_getregs((void *)&r);
    for (int i = 0; i < 32; i++) {
        assert(r.gpr[i] == cpu.gpr[i]);
    }
    assert(r.pc == cpu.pc);
    printf("%s\n", "difftest_setregs test pass!");
    assert(diffresult.wen == 0);
    assert(diffresult.pc == 0x80000018);
    printf("=========\n");
    printf("%s\n", "instn test pass!");
    printf("=========\n");
    return 0;
}
