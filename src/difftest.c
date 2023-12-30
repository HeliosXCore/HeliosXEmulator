#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "init.h"
#include "memory.h"
#include "cpu.h"

CPU_state cpu = {};
NEMUState nemu_state = {};

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

void difftest_exec(uint64_t n) { cpu_exec(n); }

void difftest_init() { init(); }
