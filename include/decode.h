#ifndef __DECODE_H__
#define __DECODE_H__

#include "cpu.h"
#include <assert.h>
#include "common.h"

extern CPU_state cpu;

static inline int check_reg_idx(int idx) {
    assert(idx >= 0 && idx < 32);
    return idx;
}

#define gpr(idx) (cpu.gpr[check_reg_idx(idx)])
#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

enum {
    TYPE_I,
    TYPE_J,
    TYPE_U,
    TYPE_R,
    TYPE_S,
    TYPE_B,
    TYPE_N,  // none
};

#define SEXT(x, len)         \
    ({                       \
        struct {             \
            int64_t n : len; \
        } __x = {.n = x};    \
        (uint64_t) __x.n;    \
    })

#define BITMASK(bits) ((1ull << (bits)) - 1)
#define BITS(x, hi, lo) \
    (((x) >> (lo)) &    \
     BITMASK((hi) - (lo) + 1))  // similar to x[hi:lo] in verilog

#define BITMASK(bits) ((1ull << (bits)) - 1)
#define BITS(x, hi, lo) \
    (((x) >> (lo)) &    \
     BITMASK((hi) - (lo) + 1))  // similar to x[hi:lo] in verilog

#define src1R()         \
    do {                \
        *src1 = R(rs1); \
    } while (0)
#define src2R()         \
    do {                \
        *src2 = R(rs2); \
    } while (0)
#define immJ()                                                       \
    do {                                                             \
        *imm = SEXT((BITS(i, 31, 31) << 20 | BITS(i, 19, 12) << 12 | \
                     BITS(i, 20, 20) << 11 | BITS(i, 30, 21) << 1),  \
                    21);                                             \
    } while (0)
#define immI()                            \
    do {                                  \
        *imm = SEXT(BITS(i, 31, 20), 12); \
    } while (0)
#define immU()                                  \
    do {                                        \
        *imm = SEXT(BITS(i, 31, 12), 20) << 12; \
    } while (0)
#define immB()                                                          \
    do {                                                                \
        *imm = SEXT(BITS(i, 31, 31) << 12 | BITS(i, 7, 7) << 11 |       \
                        BITS(i, 30, 25) << 5 | BITS(i, 11, 8) << 1 | 0, \
                    13);                                                \
    } while (0)
#define immS()                                                  \
    do {                                                        \
        *imm = SEXT(BITS(i, 31, 25) << 5 | BITS(i, 11, 7), 12); \
    } while (0)

// --- pattern matching mechanism ---
__attribute__((always_inline)) static inline void pattern_decode(
    const char *str, int len, uint64_t *key, uint64_t *mask, uint64_t *shift) {
    uint64_t __key = 0, __mask = 0, __shift = 0;
#define macro(i)                                                   \
    if ((i) >= len)                                                \
        goto finish;                                               \
    else {                                                         \
        char c = str[i];                                           \
        if (c != ' ') {                                            \
            Assert(c == '0' || c == '1' || c == '?',               \
                   "invalid character '%c' in pattern string", c); \
            __key = (__key << 1) | (c == '1' ? 1 : 0);             \
            __mask = (__mask << 1) | (c == '?' ? 0 : 1);           \
            __shift = (c == '?' ? __shift + 1 : 0);                \
        }                                                          \
    }

#define macro2(i) \
    macro(i);     \
    macro((i) + 1)
#define macro4(i) \
    macro2(i);    \
    macro2((i) + 2)
#define macro8(i) \
    macro4(i);    \
    macro4((i) + 4)
#define macro16(i) \
    macro8(i);     \
    macro8((i) + 8)
#define macro32(i) \
    macro16(i);    \
    macro16((i) + 16)
#define macro64(i) \
    macro32(i);    \
    macro32((i) + 32)
    macro64(0);
    panic("pattern too long");
#undef macro
finish:
    *key = __key >> __shift;
    *mask = __mask >> __shift;
    *shift = __shift;
}

__attribute__((always_inline)) static inline void pattern_decode_hex(
    const char *str, int len, uint64_t *key, uint64_t *mask, uint64_t *shift) {
    uint64_t __key = 0, __mask = 0, __shift = 0;
#define macro(i)                                                              \
    if ((i) >= len)                                                           \
        goto finish;                                                          \
    else {                                                                    \
        char c = str[i];                                                      \
        if (c != ' ') {                                                       \
            Assert(                                                           \
                (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || c == '?', \
                "invalid character '%c' in pattern string", c);               \
            __key = (__key << 4) | (c == '?'                 ? 0              \
                                    : (c >= '0' && c <= '9') ? c - '0'        \
                                                             : c - 'a' + 10); \
            __mask = (__mask << 4) | (c == '?' ? 0 : 0xf);                    \
            __shift = (c == '?' ? __shift + 4 : 0);                           \
        }                                                                     \
    }

    macro16(0);
    panic("pattern too long");
#undef macro
finish:
    *key = __key >> __shift;
    *mask = __mask >> __shift;
    *shift = __shift;
}

// --- pattern matching wrappers for decode ---
#define INSTPAT(pattern, ...)                                          \
    do {                                                               \
        uint64_t key, mask, shift;                                     \
        pattern_decode(pattern, STRLEN(pattern), &key, &mask, &shift); \
        if ((((uint64_t)INSTPAT_INST(s) >> shift) & mask) == key) {    \
            INSTPAT_MATCH(s, ##__VA_ARGS__);                           \
            goto *(__instpat_end);                                     \
        }                                                              \
    } while (0)

#define INSTPAT_START(name) \
    {                       \
        const void **__instpat_end = &&concat(__instpat_end_, name);
#define INSTPAT_END(name)           \
    concat(__instpat_end_, name) :; \
    }

#endif  // !__DECODE_H__
