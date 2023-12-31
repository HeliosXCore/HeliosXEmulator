#ifndef __DIFFTEST_H__
#define __DIFFTEST_H__

#include <stdint.h>
#include <stddef.h>

#define __EXPORT __attribute__((visibility("default")))

typedef struct __EXPORT _DifftestResult {
    int reg_id;
    uint32_t reg_val;
    int8_t wen;
    uint32_t pc;
} DifftestResult;

__EXPORT void difftest_memcpy_from_dut(uint32_t dest, void *src, size_t n);

__EXPORT void difftest_getregs(void *r);

__EXPORT uint32_t difftest_getreg_by_id(int reg_id);

__EXPORT void difftest_setregs(const void *r);

__EXPORT void difftest_setreg_by_id(int reg_id, uint32_t value);

__EXPORT void difftest_exec(uint64_t n, DifftestResult *difftest_result);

__EXPORT void difftest_init();

#endif  // !__DIFFTEST_H__
