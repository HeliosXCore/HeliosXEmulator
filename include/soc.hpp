#pragma once
#include "cpu.hpp"
#include "mem.hpp"
#include <memory>

namespace heliosxemulator {
    template <class Dut>
    class Soc {
       public:
        std::shared_ptr<Cpu> cpu;
        std::shared_ptr<Memory> mem;

        Soc(std::shared_ptr<Cpu> cpu, std::shared_ptr<Memory> mem)
            : cpu(cpu), mem(mem) {}

        // 对 Dut 进行仿真
        void simulate(std::shared_ptr<Dut> dut);
    };
}  // namespace heliosxemulator