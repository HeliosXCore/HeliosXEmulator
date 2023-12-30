#ifndef __DIFFTEST_H__
#define __DIFFTEST_H__

#include <stdint.h>
#include <stddef.h>

#define __EXPORT __attribute__((visibility("default")))

__EXPORT void difftest_memcpy_from_dut(uint32_t dest, void *src, size_t n);

__EXPORT void difftest_getregs(void *r);

__EXPORT void difftest_setregs(const void *r);

__EXPORT void difftest_exec(uint64_t n);

__EXPORT void difftest_init();

#endif  // !__DIFFTEST_H__
