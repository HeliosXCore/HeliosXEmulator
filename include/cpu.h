#ifndef __CPU_H__
#define __CPU_H__

#include <stdint.h>
#include "difftest.h"
#include "memory.h"

#define word_t uint32_t
#define sword_t int32_t

#define NEMUTRAP(thispc, code) set_nemu_state(NEMU_END, thispc, code)
#define INV(thispc) invalid_inst(thispc)

#define gpr(idx) (cpu.gpr[check_reg_idx(idx)])

/* const char *regs[] = {"$0", "ra", "sp",  "gp",  "tp", "t0", "t1", "t2", */
/*                       "s0", "s1", "a0",  "a1",  "a2", "a3", "a4", "a5", */
/*                       "a6", "a7", "s2",  "s3",  "s4", "s5", "s6", "s7", */
/*                       "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"}; */

typedef struct {
    // 通用寄存器
    uint32_t gpr[32];
    uint32_t pc;
} CPU_state;

// decode
typedef struct {
    union {
        uint32_t val;
    } inst;
} ISADecodeInfo;

enum { NEMU_RUNNING, NEMU_STOP, NEMU_END, NEMU_ABORT, NEMU_QUIT };

typedef struct {
    int state;
    uint32_t halt_pc;
    uint32_t halt_ret;
} NEMUState;

typedef struct Decode {
    uint32_t pc;
    uint32_t snpc;  // static next pc
    uint32_t dnpc;  // dynamic next pc
                    // 解码信息实际上就是一条32位的riscv指令
    ISADecodeInfo isa;
} Decode;

static inline uint32_t inst_fetch(uint32_t *pc, int len) {
    uint32_t inst = vaddr_ifetch(*pc, len);
    (*pc) += len;
    return inst;
}
void invalid_inst(uint32_t thispc);

void set_nemu_state(int state, uint32_t pc, int halt_ret);
void invalid_inst(uint32_t thispc);

void init_isa();

void cpu_exec(uint64_t n, DifftestResult *difftest_result);

#endif
