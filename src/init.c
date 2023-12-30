#include "init.h"
#include <time.h>
#include <stdlib.h>
#include "common.h"
#include "memory.h"
#include "cpu.h"

static void init_rand() { srand(time(NULL)); }
static void init_log() { Log("%s\n", "HeliosXEmulator run!"); }

void init() {
    init_rand();

    init_log();

    init_mem();

    init_isa();
}
