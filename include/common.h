#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>

#if !defined(likely)
#define likely(cond) __builtin_expect(cond, 1)
#define unlikely(cond) __builtin_expect(cond, 0)
#endif

#define _Log(...)            \
    do {                     \
        printf(__VA_ARGS__); \
    } while (0)

#define Assert(cond, format, ...)                                \
    do {                                                         \
        if (!(cond)) {                                           \
            (fflush(stdout),                                     \
             fprintf(stderr, ANSI_FMT(format, ANSI_FG_RED) "\n", \
                     ##__VA_ARGS__));                            \
            extern void assert_fail_msg();                       \
            assert(cond);                                        \
        }                                                        \
    } while (0)

#define panic(format, ...) Assert(0, format, ##__VA_ARGS__)

#define STRLEN(CONST_STR) (sizeof(CONST_STR) - 1)

#define concat_temp(x, y) x##y
#define concat(x, y) concat_temp(x, y)
#define concat3(x, y, z) concat(concat(x, y), z)
#define concat4(x, y, z, w) concat3(concat(x, y), z, w)
#define concat5(x, y, z, v, w) concat4(concat(x, y), z, v, w)

#define ANSI_FG_BLACK "\33[1;30m"
#define ANSI_FG_RED "\33[1;31m"
#define ANSI_FG_GREEN "\33[1;32m"
#define ANSI_FG_YELLOW "\33[1;33m"
#define ANSI_FG_BLUE "\33[1;34m"
#define ANSI_FG_MAGENTA "\33[1;35m"
#define ANSI_FG_CYAN "\33[1;36m"
#define ANSI_FG_WHITE "\33[1;37m"
// 40–47	Set background color
#define ANSI_BG_BLACK "\33[1;40m"
#define ANSI_BG_RED "\33[1;41m"
#define ANSI_BG_GREEN "\33[1;42m"
#define ANSI_BG_YELLOW "\33[1;43m"
#define ANSI_BG_BLUE "\33[1;44m"
#define ANSI_BG_MAGENTA "\33[1;35m"
#define ANSI_BG_CYAN "\33[1;46m"
#define ANSI_BG_WHITE "\33[1;47m"
#define ANSI_NONE "\33[0m"
// define ANSI escape code printf
#define ANSI_FMT(str, fmt) fmt str ANSI_NONE

#define FMT_PADDR "0x%08x"
#define FMT_WORD FMT_PADDR

#define Log(format, ...)                                              \
    _Log(ANSI_FMT("[%s:%d %s] " format, ANSI_FG_BLUE) "\n", __FILE__, \
         __LINE__, __func__, ##__VA_ARGS__)

#endif
