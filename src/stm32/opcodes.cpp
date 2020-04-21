// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "opcodes.hpp"

#include <cassert>

#include "math.hpp"

namespace stm32::opcodes::t1
{
void cmd_lsl(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
    if (!registers.conditionPassed())
    {
        return;
    }

    const auto d = registers.reg(opCode);
    const auto m = registers.reg(opCode >> 3u);
    const auto shift = math::decodeImmediateShift(0b00u, opCode >> 6u).second;
}

void cmd_lsr(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
}

void cmd_asr(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
}

void cmd_add(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
}

void cmd_sub(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
}

void cmd_mov(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
}

void cmd_cmp(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
}

}  // namespace stm32::opcodes::t1
