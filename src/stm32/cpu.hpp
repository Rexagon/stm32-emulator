#pragma once

#include "cpu_register_set.hpp"
#include "memory.hpp"

namespace stm32
{
class VirtualCpu
{
public:
    void reset();
    void step();

private:
    CpuRegisterSet m_registers;
    Memory m_memory;
};
}
