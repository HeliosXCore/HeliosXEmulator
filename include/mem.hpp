#pragma once
#include <vector>

namespace heliosxemulator {
    class Memory {
       public:
        std::vector<uint8_t> mem;

        void load_elf(std::shared_ptr<uint8_t> ptr, size_t size);
    };
}  // namespace heliosxemulator