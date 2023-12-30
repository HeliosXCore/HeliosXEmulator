# HeliosXEmulator
HeliosXEmulator is a RISC-V emulator written in C++ for verilator simulation.

HeliosXEmulator refered to [NEMU](https://github.com/NJU-ProjectN/nemu).

To get a shared lib,execute this command `make share`

## Introduction to Function Interface

```c
uint32_t difftest_getreg_by_id(int regId);
```
> regId: 0~31 is the number of the general register ; 32 is the number of the PC

Here are some structs that might be used in the simulator
```c
typedef struct {
  // 通用寄存器
  uint32_t gpr[32];
  uint32_t pc;
} CPU_state;
typedef struct __attribute__((packed)) _DifftestResult {
  int reg_id;
  uint32_t reg_val;
  int8_t wen;
  uint32_t pc;
} DifftestResult;
```
