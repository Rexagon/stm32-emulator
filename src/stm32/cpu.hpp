#pragma once

#include "opcodes.hpp"

namespace stm32 {
class VirtualCpu {
public:
    void reset();
    void step();

private:
    CpuRegisterSet m_registers;
    Memory m_memory;
};
}  // namespace stm32
