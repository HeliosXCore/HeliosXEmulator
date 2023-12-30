#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>
#include "cpu.h"
#include "difftest.h"
#include "memory.h"

extern CPU_state cpu;
extern NEMUState nemu_state;

int main(int argc, char *argv[]) {
    difftest_init();

    const uint32_t img[] = {
        0x00000297,  // auipc t0,0
        0x00028823,  // sb  zero,16(t0)
        0x0102c503,  // lbu a0,16(t0)
        0x00100073,  // ebreak (used as nemu_trap)
        0xdeadbeef,  // some data
    };

    difftest_memcpy_from_dut(RESET_VECTOR, (uint32_t *)img, sizeof(img));
    difftest_exec(1);
    CPU_state r;
    difftest_getregs((void *)&r);
    for (int i = 0; i < 32; i++) {
        assert(r.gpr[i] == cpu.gpr[i]);
    }
    assert(r.pc == cpu.pc);
    printf("%s\n", "difftest_getregs test pass!");

    r.pc = RESET_VECTOR;
    difftest_setregs((void *)&r);
    difftest_exec(-1);
    difftest_getregs((void *)&r);
    for (int i = 0; i < 32; i++) {
        assert(r.gpr[i] == cpu.gpr[i]);
    }
    assert(r.pc == cpu.pc);
    printf("%s\n", "difftest_setregs test pass!");
    return 0;
}
