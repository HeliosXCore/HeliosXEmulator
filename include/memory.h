#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <common.h>
#include <stdint.h>
#include <stdbool.h>
#include_next <assert.h>

#define PG_ALIGN __attribute((aligned(4096)))

#define CONFIG_PC_RESET_OFFSET 0x0
#define CONFIG_MBASE 0x80000000
#define CONFIG_MSIZE 0x8000000

// 内存起始地址:2GB
#define PMEM_LEFT ((uint32_t)CONFIG_MBASE)
// 内存终止地址:2GB+128MB
#define PMEM_RIGHT ((uint32_t)CONFIG_MBASE + CONFIG_MSIZE - 1)

#define RESET_VECTOR (PMEM_LEFT + CONFIG_PC_RESET_OFFSET)

uint8_t *guest_to_host(uint32_t paddr);
uint32_t host_to_guest(uint8_t *haddr);

static inline uint32_t host_read(void *addr, int len) {
    switch (len) {
        case 1:
            return *(uint8_t *)addr;
        case 2:
            return *(uint16_t *)addr;
        case 4:
            return *(uint32_t *)addr;
        default:
            return 0;
    }
}

static inline void host_write(void *addr, int len, uint32_t data) {
    switch (len) {
        case 1:
            *(uint8_t *)addr = data;
            return;
        case 2:
            *(uint16_t *)addr = data;
            return;
        case 4:
            *(uint32_t *)addr = data;
            return;
        default:
            assert(0);
    }
}

uint32_t paddr_read(uint32_t addr, int len);

void paddr_write(uint32_t addr, int len, uint32_t data);

uint32_t vaddr_read(uint32_t addr, int len);

void vaddr_write(uint32_t addr, int len, uint32_t data);

void init_mem();

uint32_t vaddr_ifetch(uint32_t addr, int len);

#endif
