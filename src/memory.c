#include <stdint.h>
#include "memory.h"
#include <stdlib.h>
#include "common.h"

static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};

uint8_t *guest_to_host(uint32_t paddr) { return pmem + paddr - CONFIG_MBASE; }
uint32_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }

static inline bool in_pmem(uint32_t addr) {
    return addr - CONFIG_MBASE < CONFIG_MSIZE;
}

uint32_t vaddr_ifetch(uint32_t addr, int len) { return paddr_read(addr, len); }

static uint32_t pmem_read(uint32_t addr, int len) {
    uint32_t ret = host_read(guest_to_host(addr), len);
    return ret;
}

static void pmem_write(uint32_t addr, int len, uint32_t data) {
    host_write(guest_to_host(addr), len, data);
}

uint32_t paddr_read(uint32_t addr, int len) {
    if (likely(in_pmem(addr))) return pmem_read(addr, len);
    return 0;
}

void paddr_write(uint32_t addr, int len, uint32_t data) {
    if (likely(in_pmem(addr))) {
        pmem_write(addr, len, data);
        return;
    }
}

uint32_t vaddr_read(uint32_t addr, int len) { return paddr_read(addr, len); }

void vaddr_write(uint32_t addr, int len, uint32_t data) {
    paddr_write(addr, len, data);
}

void init_mem() {
    uint32_t *p = (uint32_t *)pmem;
    int i;
    // 用随机数填充nemu的内存
    for (i = 0; i < (int)(CONFIG_MSIZE / sizeof(p[0])); i++) {
        p[i] = rand();
    }
    // TODO
    Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", PMEM_LEFT,
        PMEM_RIGHT);
}
