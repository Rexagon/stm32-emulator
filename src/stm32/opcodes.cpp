// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "opcodes.hpp"

#include <cassert>

namespace stm32::opcodes
{

void cmd_mov(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
    if (!registers.conditionPassed())
    {
        return;
    }

    const auto& imm8 = opCode & 0b11111111u;
    auto& Rd = registers.reg(opCode >> 8u);

    auto& APSR = registers.APSR();

    const auto imm32 = static_cast<uint32_t>(imm8);

    Rd = imm32;
    if (registers.isInItBlock())
    {
        APSR.N = imm32 & math::LEFT_BIT<uint32_t>;
        APSR.Z = imm32 == 0;
    }
}

void cmd_cmp(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
    if (!registers.conditionPassed())
    {
        return;
    }

    const auto& imm8 = opCode & 0b11111111u;
    const auto& Rn = registers.reg(opCode >> 8u);

    auto& APSR = registers.APSR();

    const auto imm32 = static_cast<uint32_t>(imm8);

    const auto [result, carry, overflow] = math::addWithCarry(Rn, ~imm32, true);

    APSR.N = imm32 & math::LEFT_BIT<uint32_t>;
    APSR.Z = imm32 == 0;
    APSR.C = carry;
    APSR.V = overflow;
}

}  // namespace stm32::opcodes
