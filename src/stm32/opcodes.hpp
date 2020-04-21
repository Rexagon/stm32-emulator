#pragma once

#include "cpu_register_set.hpp"
#include "math.hpp"
#include "memory.hpp"
#include "utils.hpp"

namespace stm32::opcodes {
enum class Encoding { T1, T2, T3, T4 };

template <auto v, auto... vs>
constexpr bool is_in = ((v == vs) || ...);

template <Encoding TargetEnc, Encoding Enc, typename TargetOpCodeType, typename OpCodeType>
constexpr bool is_valid_opcode_encoding = (Enc == TargetEnc) && std::is_same_v<OpCodeType, TargetOpCodeType>;

template <uint8_t /* offset */, uint8_t /* bitCount */, typename T = uint8_t>
struct Part {
};

template <typename, typename>
struct Extractor;

template <typename Op, uint8_t offset, uint8_t bitCount, typename T>
struct Extractor<Op, Part<offset, bitCount, T>> {
    using Result = T;
    static constexpr auto extract(const Op& opCode) -> T { return static_cast<T>(opCode >> offset) & math::BITS<bitCount, T>; }
};

template <typename Op, typename... Parts>
auto splitOpCode(const Op& opCode) -> std::tuple<typename Extractor<Op, Parts>::Result...>
{
    return std::tuple(Extractor<Op, Parts>::extract(opCode)...);
}

template <Encoding encoding, math::ShiftType shiftType, typename T>
inline void cmdShiftImmediate(T opCode, CpuRegisterSet& registers, Memory& memory)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);
    static_assert(is_in<shiftType, math::ShiftType::LSL, math::ShiftType::LSR, math::ShiftType::ASR>);

    if (!registers.conditionPassed()) {
        return;
    }

    auto& APSR = registers.APSR();

    uint8_t d, m, setFlags, shiftN;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rd, Rm, imm5, shiftTypeBits] = splitOpCode<T, Part<0, 3>, Part<3, 3>, Part<6, 5>, Part<11, 2>>(opCode);

        d = Rd;
        m = Rm;
        setFlags = !registers.isInItBlock();
        shiftN = math::decodeImmediateShift(shiftTypeBits, imm5).second;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, imm2, Rd, imm3, S, shiftTypeBits] =
            splitOpCode<T, Part<0, 4>, Part<4, 2>, Part<6, 2>, Part<8, 4>, Part<12, 3>, Part<20, 1>>(opCode);

        d = Rd;
        m = Rm;
        setFlags = S;
        shiftN = math::decodeImmediateShift(shiftTypeBits, (imm3 << 2u) | imm2).second;

        assert(d < 13 && m < 13);
    }

    auto& Rd = registers.reg(d);
    const auto& Rm = registers.reg(m);

    const auto [result, carry] = math::shiftCarry(Rm, shiftType, shiftN, APSR.C);
    Rd = result;

    if (setFlags) {
        APSR.N = result & math::LEFT_BIT<uint32_t>;
        APSR.Z = result == 0;
        APSR.C = carry;
    }
}

template <Encoding encoding, math::ShiftType shiftType, typename T>
inline void cmdShiftRegister(T opCode, CpuRegisterSet& registers, Memory& memory)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);
    static_assert(is_in<shiftType, math::ShiftType::LSL, math::ShiftType::LSR, math::ShiftType::ASR>);

    if (!registers.conditionPassed()) {
        return;
    }

    auto& APSR = registers.APSR();

    uint8_t d, n, m, setFlags;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rdn, Rm] = splitOpCode<T, Part<0, 3>, Part<3, 3>>(opCode);

        d = Rdn;
        n = Rdn;
        m = Rm;
        setFlags = !registers.isInItBlock();
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, Rd, Rn, S] = splitOpCode<T, Part<0, 4>, Part<8, 4>, Part<16, 4>, Part<20, 1>>(opCode);

        d = Rd;
        n = Rn;
        m = Rm;
        setFlags = S;

        assert(d < 13 && n < 13 && m < 13);
    }

    const auto shiftN = math::maskedShift<0, 8>(registers.reg(m));
    const auto& Rn = registers.reg(n);
    auto& Rd = registers.reg(d);

    const auto [result, carry] = math::shiftCarry(Rn, shiftType, shiftN, APSR.C);
    Rd = result;

    if (setFlags) {
        APSR.N = result & math::LEFT_BIT<uint32_t>;
        APSR.Z = result == 0;
        APSR.C = carry;
    }
}

template <Encoding encoding, bool isSub, typename T>
void cmdAddSubImmediate(T opCode, CpuRegisterSet& registers, Memory& memory)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2, Encoding::T3, Encoding::T4>);

    if (!registers.conditionPassed()) {
        return;
    }

    auto& APSR = registers.APSR();

    uint8_t d, n, setFlags;
    uint32_t imm32;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rd, Rn, imm3] = splitOpCode<T, Part<0, 3>, Part<3, 3>, Part<6, 3, uint32_t>>(opCode);

        d = Rd;
        n = Rn;
        setFlags = !registers.isInItBlock();
        imm32 = imm3;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint16_t, T>) {
        const auto [imm8, Rdn] = splitOpCode<T, Part<0, 8, uint32_t>, Part<8, 3>>(opCode);

        d = Rdn;
        n = Rdn;
        setFlags = !registers.isInItBlock();
        imm32 = imm8;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>) {
        const auto [imm8, Rd, imm3, Rn, S, i] =
            splitOpCode<T, Part<0, 8, uint32_t>, Part<8, 4>, Part<12, 3, uint32_t>, Part<16, 4>, Part<20, 1>, Part<26, 1, uint32_t>>(
                opCode);

        d = Rd;
        n = Rn;
        setFlags = S;
        imm32 = (i << 11u) | (imm3 << 8u) | imm8;  // TODO: see ThumbExpandImm
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T4, encoding, uint32_t, T>) {
        const auto [imm8, Rd, imm3, Rn, i] =
            splitOpCode<T, Part<0, 8, uint32_t>, Part<8, 4>, Part<12, 3, uint32_t>, Part<16, 4>, Part<26, 1, uint32_t>>(opCode);

        d = Rd;
        n = Rn;
        setFlags = 0;
        imm32 = (i << 11u) | (imm3 << 8u) | imm8;  // TODO: see ZeroExtend(..., 32)
    }

    const auto& Rn = registers.reg(n);
    auto& Rd = registers.reg(d);

    const auto [result, carry, overflow] = math::addWithCarry(Rd, imm32, false);

    Rd = result;
    if (setFlags) {
        APSR.N = math::isNegative(result);
        APSR.Z = result == 0;
        APSR.C = carry;
        APSR.V = overflow;
    }
}

void cmd_mov(uint16_t opCode, CpuRegisterSet& registers, Memory& memory);
void cmd_cmp(uint16_t opCode, CpuRegisterSet& registers, Memory& memory);

}  // namespace stm32::opcodes