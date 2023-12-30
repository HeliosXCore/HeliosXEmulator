#include "difftest.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "init.h"
#include "memory.h"
#include "cpu.h"

CPU_state cpu = {};
NEMUState nemu_state = {};

void difftest_setreg_by_id(int regId, uint32_t value) {
    if (regId == 32) {
        cpu.pc = value;
    } else if (regId >= 0 && regId <= 31) {
        cpu.gpr[regId] = value;
    } else {
        printf("\n%s\n", "=======regId is not in range [0-32]======");
    }
}

uint32_t difftest_getreg_by_id(int regId) {
    if (regId == 32) {
        return cpu.pc;
    } else if (regId >= 0 && regId <= 31) {
        return cpu.gpr[regId];
    } else {
        printf("\n%s\n", "=======regId is not in range [0-32]======");
        return -1;
    }
}

void difftest_memcpy_from_dut(uint32_t dest, void *src, size_t n) {
    uint8_t *data = (uint8_t *)malloc(sizeof(uint8_t));
    while (n > 0) {
        memcpy(data, src, 1);
        paddr_write(dest, 1, *data);
        src++;
        dest++;
        n--;
    }
}

void difftest_getregs(void *r) { memcpy(r, &cpu, sizeof(uint32_t) * 33); }

void difftest_setregs(const void *r) { memcpy(&cpu, r, sizeof(uint32_t) * 33); }

void difftest_exec(uint64_t n, DifftestResult *difftest_result) {
    if (n > 1) {
        printf("%s\n",
               "==========just the last inst dst register will be "
               "record,because n>1===========");
    }
    cpu_exec(n, difftest_result);
}

void difftest_init() { init(); }
