// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "opcodes.hpp"

#include <cassert>

namespace
{
template <typename T>
constexpr bool LEFT_BIT = T(0b1u) << (sizeof(T) * 8u - 1u);

template <typename T>
inline auto lsl_c(T x, uint8_t shift) -> std::pair<T, bool>
{
    assert(shift > 0);
    const auto carry = x & LEFT_BIT<T>;
    return {x << shift, carry};
}

inline auto lsl(uint32_t x, uint8_t shift) -> uint32_t
{
    return x << shift;
}

enum class ShiftType
{
    LSL,
    LSR,
    ASR,
    RRX,
    ROR,
};

auto decode_imm_shift(uint8_t bits, uint16_t immediate) -> std::pair<ShiftType, uint8_t>
{
    switch (bits)
    {
        case 0b00u:
            return {ShiftType::LSL, immediate};
        case 0b01u:
            switch (immediate)
            {
                case 0b00000u:
                    return {ShiftType::LSR, 0b11111u};
                default:
                    return {ShiftType::LSR, immediate};
            }
        case 0b10u:
            switch (immediate)
            {
                case 0b00000u:
                    return {ShiftType::ASR, 0b11111u};
                default:
                    return {ShiftType::ASR, immediate};
            }
        case 0b11u:
            switch (immediate)
            {
                case 0b00000u:
                    return {ShiftType::RRX, 0b1u};
                default:
                    return {ShiftType::ROR, immediate};
            }
        default:
            assert("UNPREDICTABLE");
            break;
    }
}

auto shift_carry(uint32_t value, ShiftType shiftType, uint8_t amount, bool carry_in) -> std::pair<uint32_t, bool>
{
    assert(shiftType != ShiftType::RRX || amount == 1u);

    if (amount == 0)
    {
        return {value, carry_in};
    }

    switch (shiftType)
    {
        case ShiftType::LSL:
            break;
        case ShiftType::LSR:
            break;
        case ShiftType::ASR:
            break;
        case ShiftType::RRX:
            break;
        case ShiftType::ROR:
            break;
    }
}

}  // namespace

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
    const auto shift = decode_imm_shift(0b00u, opCode >> 6u).second;
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
