#pragma once

#include "cpu_register_set.hpp"
#include "math.hpp"
#include "memory.hpp"
#include "system_control_registers.hpp"
#include "utils.hpp"

namespace stm32::opcodes
{
template <uint8_t offset, uint8_t bitCount, typename T = uint8_t>
using Part = math::Part<offset, bitCount, T>;

enum class Encoding { T1, T2, T3, T4 };

enum class Bitwise { AND, EOR, ORR, BIC };

template <auto v, auto... vs>
constexpr bool is_in = ((v == vs) || ...);

template <Encoding TargetEnc, Encoding Enc, typename TargetOpCodeType, typename OpCodeType>
constexpr bool is_valid_opcode_encoding = (Enc == TargetEnc) && std::is_same_v<OpCodeType, TargetOpCodeType>;

template <bool b>
constexpr bool check = b;  // Clion unused code highlighting fix

template <Encoding encoding, math::ShiftType shiftType, typename T>
inline void cmdShiftImmediate(T opCode, CpuRegisterSet& registers)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);
    static_assert(is_in<shiftType, math::ShiftType::LSL, math::ShiftType::LSR, math::ShiftType::ASR>);

    if (!registers.conditionPassed()) {
        return;
    }

    auto& APSR = registers.APSR();

    uint8_t d, m, setFlags, shiftN;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rd, Rm, imm5, shiftTypeBits] = math::split<T, Part<0, 3>, Part<3, 3>, Part<6, 5>, Part<11, 2>>(opCode);

        d = Rd;
        m = Rm;
        setFlags = !registers.isInItBlock();
        shiftN = math::decodeImmediateShift(shiftTypeBits, imm5).second;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, imm2, Rd, imm3, S, shiftTypeBits] =
            math::split<T, Part<0, 4>, Part<6, 2>, Part<8, 4>, Part<12, 3>, Part<20, 1>>(opCode);

        assert(Rd < 13 && Rm < 13);

        d = Rd;
        m = Rm;
        setFlags = S;
        shiftN = math::decodeImmediateShift(shiftTypeBits, math::combine<uint8_t>(Part<0, 2>{imm2}, Part<2, 3>{imm3})).second;
    }

    auto& Rd = registers.reg(d);
    const auto& Rm = registers.reg(m);

    const auto [result, carry] = math::shiftWithCarry(Rm, shiftType, shiftN, APSR.C);
    Rd = result;

    if (setFlags) {
        APSR.N = result & math::LEFT_BIT<uint32_t>;
        APSR.Z = result == 0;
        APSR.C = carry;
    }
}

template <Encoding encoding, math::ShiftType shiftType, typename T>
inline void cmdShiftRegister(T opCode, CpuRegisterSet& registers)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);
    static_assert(is_in<shiftType, math::ShiftType::LSL, math::ShiftType::LSR, math::ShiftType::ASR, math::ShiftType::ROR>);

    if (!registers.conditionPassed()) {
        return;
    }

    auto& APSR = registers.APSR();

    uint8_t d, n, m, setFlags;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rdn, Rm] = math::split<T, Part<0, 3>, Part<3, 3>>(opCode);

        d = Rdn;
        n = Rdn;
        m = Rm;
        setFlags = !registers.isInItBlock();
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, Rd, Rn, S] = math::split<T, Part<0, 4>, Part<8, 4>, Part<16, 4>, Part<20, 1>>(opCode);

        assert(Rd < 13 && Rn < 13 && Rm < 13);

        d = Rd;
        n = Rn;
        m = Rm;
        setFlags = S;
    }

    const auto shiftN = math::getPart<0, 8, uint32_t, uint8_t>(registers.reg(m));
    const auto& Rn = registers.reg(n);
    auto& Rd = registers.reg(d);

    const auto [result, carry] = math::shiftWithCarry(Rn, shiftType, shiftN, APSR.C);
    Rd = result;

    if (setFlags) {
        APSR.N = result & math::LEFT_BIT<uint32_t>;
        APSR.Z = result == 0;
        APSR.C = carry;
    }
}

template <Encoding encoding, bool isSub, typename T>
void cmdAddSubImmediate(T opCode, CpuRegisterSet& registers)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2, Encoding::T3, Encoding::T4>);

    if (!registers.conditionPassed()) {
        return;
    }

    auto& APSR = registers.APSR();

    uint8_t d, n, setFlags;
    uint32_t imm32;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rd, Rn, imm3] = math::split<T, Part<0, 3>, Part<3, 3>, Part<6, 3, uint32_t>>(opCode);

        d = Rd;
        n = Rn;
        setFlags = !registers.isInItBlock();
        imm32 = imm3;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint16_t, T>) {
        const auto [imm8, Rdn] = math::split<T, Part<0, 8, uint32_t>, Part<8, 3>>(opCode);

        d = Rdn;
        n = Rdn;
        setFlags = !registers.isInItBlock();
        imm32 = imm8;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>) {
        const auto [imm8, Rd, imm3, Rn, S, i] =
            math::split<T, Part<0, 8>, Part<8, 4>, Part<12, 3>, Part<16, 4>, Part<20, 1>, Part<26, 1>>(opCode);

        d = Rd;
        n = Rn;
        setFlags = S;

        const auto combined = math::combine<uint16_t>(Part<0, 8>{imm8}, Part<8, 3>{imm3}, Part<12, 1>{i});
        imm32 = math::thumbExpandImmediateWithCarry(combined, APSR.C).first;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T4, encoding, uint32_t, T>) {
        const auto [imm8, Rd, imm3, Rn, i] = math::split<T, Part<0, 8>, Part<8, 4>, Part<12, 3>, Part<16, 4>, Part<26, 1>>(opCode);

        d = Rd;
        n = Rn;
        setFlags = 0;
        imm32 = math::combine<uint32_t>(Part<0, 8>{imm8}, Part<8, 3>{imm3}, Part<12, 1>{i});
    }

    const auto& Rn = registers.reg(n);
    auto& Rd = registers.reg(d);

    if constexpr (check<isSub>) {
        imm32 = ~imm32;
    }

    const auto [result, carry, overflow] = math::addWithCarry(Rn, imm32, isSub);

    Rd = result;
    if (setFlags) {
        APSR.N = math::isNegative(result);
        APSR.Z = result == 0;
        APSR.C = carry;
        APSR.V = overflow;
    }
}

template <Encoding encoding, bool isSub, typename T>
void cmdAddSubRegister(T opCode, CpuRegisterSet& registers)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2> || (encoding == Encoding::T3 && !isSub));

    if (!registers.conditionPassed()) {
        return;
    }

    auto& APSR = registers.APSR();

    uint8_t d, n, setFlags;
    uint32_t shifted;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rd, Rn, Rm] = math::split<T, Part<0, 3>, Part<3, 3>, Part<6, 3>>(opCode);

        d = Rd;
        n = Rn;
        setFlags = !registers.isInItBlock();
        shifted = registers.reg(Rm);
    }
    else if constexpr (check<!isSub> && is_valid_opcode_encoding<Encoding::T2, encoding, uint16_t, T>) {
        const auto [Rdn, Rm, DN] = math::split<T, Part<0, 3>, Part<3, 4>, Part<7, 1>>(opCode);

        d = math::combine<uint8_t>(Part<0, 3>{Rdn}, Part<3, 1>{DN});
        assert(d != 15 || !registers.isInItBlock() || registers.isLastInItBlock());
        assert(d != 15 || Rm != 15);

        n = d;
        setFlags = false;
        shifted = registers.reg(Rm);
    }
    else if constexpr ((check<isSub> && is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>) ||
                       (check<!isSub> && is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>)) {
        const auto [Rm, type, imm2, Rd, imm3, Rn, S] =
            math::split<T, Part<0, 4>, Part<4, 2>, Part<6, 2>, Part<8, 4>, Part<12, 3>, Part<16, 4>, Part<20, 1>>(opCode);

        assert(Rd != 13 && (Rd != 15 || S != 0) && Rn != 15 && Rm < 13);

        const auto [shiftType, shiftN] = math::decodeImmediateShift(type, math::combine<uint8_t>(Part<0, 2>{imm2}, Part<2, 3>{imm3}));

        d = Rd;
        n = Rn;
        setFlags = S;
        shifted = math::shift(registers.reg(Rm), shiftType, shiftN, APSR.C);
    }

    const auto& Rn = registers.reg(n);
    auto& Rd = registers.reg(d);

    if constexpr (check<isSub>) {
        shifted = ~shifted;
    }

    const auto [result, carry, overflow] = math::addWithCarry(Rn, shifted, isSub);  // TODO: verify if isSub is true for SUB
                                                                                    // (maybe Clion is right)

    if (check<!isSub> && d == 15) {
        registers.aluWritePC(result);
    }
    else {
        Rd = result;
        if (setFlags) {
            APSR.N = math::isNegative(result);
            APSR.Z = result == 0;
            APSR.C = carry;
            APSR.V = overflow;
        }
    }
}

template <Encoding encoding, bool isSbc, typename T>
void cmdAdcSbcRegister(T opCode, CpuRegisterSet& registers)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    if (!registers.conditionPassed()) {
        return;
    }

    auto& APSR = registers.APSR();

    uint8_t d, n, setFlags;
    uint32_t shifted;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rdn, Rm] = math::split<T, Part<0, 3>, Part<3, 3>>(opCode);

        d = Rdn;
        n = Rdn;
        setFlags = !registers.isInItBlock();
        shifted = registers.reg(Rm);
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, type, imm2, Rd, imm3, Rn, S] =
            math::split<T, Part<0, 4>, Part<4, 2>, Part<8, 4>, Part<12, 3>, Part<16, 4>, Part<20, 1>>(opCode);

        assert(Rd < 13 && Rn < 13 && Rm < 13);

        const auto [shiftType, shiftN] = math::decodeImmediateShift(type, math::combine<uint8_t>(Part<0, 2>{imm2}, Part<2, 3>{imm3}));

        d = Rd;
        n = Rn;
        shifted = math::shift(registers.reg(Rm), shiftType, shiftN, APSR.C);
    }

    const auto& Rn = registers.reg(n);
    auto& Rd = registers.reg(d);

    if constexpr (check<isSbc>) {
        shifted = ~shifted;
    }

    const auto [result, carry, overflow] = math::addWithCarry(Rn, shifted, isSbc);

    Rd = result;
    if (setFlags) {
        APSR.N = math::isNegative(result);
        APSR.Z = result == 0;
        APSR.C = carry;
        APSR.V = overflow;
    }
}

template <Encoding encoding, typename T>
void cmdRsbImmediate(T opCode, CpuRegisterSet& registers)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    if (!registers.conditionPassed()) {
        return;
    }

    auto& APSR = registers.APSR();

    uint8_t d, n, setFlags;
    uint32_t imm32;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rd, Rn] = math::split<T, Part<0, 3>, Part<3, 3>>(opCode);

        d = Rd;
        n = Rn;
        setFlags = !registers.isInItBlock();
        imm32 = 0x0u;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [imm8, Rd, imm3, Rn, S, i] =
            math::split<T, Part<0, 8>, Part<8, 4>, Part<12, 3>, Part<16, 4>, Part<20, 1>, Part<26, 1>>(opCode);

        assert(Rd < 13 && Rn < 13);

        d = Rd;
        n = Rn;
        setFlags = S;
        imm32 =
            math::thumbExpandImmediateWithCarry(math::combine<uint32_t>(Part<0, 8>{imm8}, Part<8, 3>{imm3}, Part<12, 1>{i}), APSR.C).first;
    }

    const auto& Rn = registers.reg(n);
    auto& Rd = registers.reg(d);

    const auto [result, carry, overflow] = math::addWithCarry(~Rn, imm32, true);

    Rd = result;
    if (setFlags) {
        APSR.N = math::isNegative(result);
        APSR.Z = result == 0;
        APSR.C = carry;
        APSR.V = overflow;
    }
}

template <Encoding encoding, typename T>
void cmdMul(T opCode, CpuRegisterSet& registers)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    if (!registers.conditionPassed()) {
        return;
    }

    auto& APSR = registers.APSR();

    uint8_t d, n, m, setFlags;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rdm, Rn] = math::split<T, Part<0, 3>, Part<3, 3>>(opCode);

        d = Rdm;
        n = Rn;
        m = Rdm;
        setFlags = !registers.isInItBlock();
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, Rd, Rn] = math::split<T, Part<0, 4>, Part<8, 4>, Part<16, 4>>(opCode);
        assert(Rd < 13 && n < 13 && m < 13);

        d = Rd;
        n = Rn;
        m = Rm;
        setFlags = false;
    }

    const auto& Rn = registers.reg(n);
    const auto& Rm = registers.reg(m);
    auto& Rd = registers.reg(d);

    const auto result = Rn * Rm;

    Rd = result;
    if (setFlags) {
        APSR.N = math::isNegative(result);
        APSR.Z = result == 0;
    }
}

template <Encoding encoding, Bitwise bitwise, typename T>
void cmdBitwiseRegister(T opCode, CpuRegisterSet& registers)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    if (!registers.conditionPassed()) {
        return;
    }

    auto& APSR = registers.APSR();

    uint8_t d, n, setFlags;
    uint32_t shifted;
    bool carry;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rdn, Rm] = math::split<T, Part<0, 3>, Part<3, 3>>(opCode);

        d = Rdn;
        n = Rdn;
        setFlags = !registers.isInItBlock();

        shifted = registers.reg(Rm);
        carry = APSR.C;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, type, imm2, Rd, imm3, Rn, S] =
            math::split<T, Part<0, 4>, Part<4, 2>, Part<6, 2>, Part<8, 4>, Part<12, 3>, Part<16, 4>, Part<20, 1>>(opCode);

        const auto [shiftType, shiftN] = math::decodeImmediateShift(type, math::combine<uint8_t>(Part<0, 2>{imm2}, Part<2, 3>{imm3}));

        if constexpr (is_in<bitwise, Bitwise::AND, Bitwise::EOR>) {
            assert(Rd != 13 && (Rd != 15 || S != 0) && Rn < 13 && Rm < 13);
        }
        else if constexpr (is_in<bitwise, Bitwise::ORR>) {
            assert(Rd < 13 && Rn != 13 && Rm < 13);
        }
        else if constexpr (is_in<bitwise, Bitwise::BIC>) {
            assert(Rd < 13 && Rn < 13 && Rm < 13);
        }

        d = Rd;
        n = Rn;
        setFlags = S;

        std::tie(shifted, carry) = math::shiftWithCarry(registers.reg(Rm), shiftType, shiftN, APSR.C);
    }

    const auto& Rn = registers.reg(n);
    auto& Rd = registers.reg(d);

    uint32_t result;
    if constexpr (is_in<bitwise, Bitwise::AND>) {
        result = Rn & shifted;
    }
    else if constexpr (is_in<bitwise, Bitwise::EOR>) {
        result = Rn ^ shifted;
    }
    else if constexpr (is_in<bitwise, Bitwise::ORR>) {
        result = Rn | shifted;
    }
    else if constexpr (is_in<bitwise, Bitwise::BIC>) {
        result = Rn & ~shifted;
    }

    Rd = result;
    if (setFlags) {
        APSR.N = math::isNegative(result);
        APSR.Z = result == 0;
        APSR.C = carry;
    }
}

template <Encoding encoding, typename T>
void cmdMvnRegister(T opCode, CpuRegisterSet& registers)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    if (!registers.conditionPassed()) {
        return;
    }

    auto& APSR = registers.APSR();

    uint8_t d, setFlags;
    uint32_t shifted;
    bool carry;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rd, Rm] = math::split<T, Part<0, 3>, Part<3, 3>>(opCode);

        d = Rd;
        setFlags = !registers.isInItBlock();

        shifted = registers.reg(Rm);
        carry = APSR.C;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, type, imm2, Rd, imm3, S] =
            math::split<T, Part<0, 4>, Part<4, 2>, Part<6, 2>, Part<8, 4>, Part<12, 3>, Part<20, 1>>(opCode);

        assert(Rd < 13 && Rm < 13);

        const auto [shiftType, shiftN] = math::decodeImmediateShift(type, math::combine<uint8_t>(Part<0, 2>{imm2}, Part<2, 3>{imm3}));

        d = Rd;
        setFlags = S;

        std::tie(shifted, carry) = math::shiftWithCarry(registers.reg(Rm), shiftType, shiftN, APSR.C);
    }

    auto& Rd = registers.reg(d);

    const auto result = ~shifted;

    Rd = result;
    if (setFlags) {
        APSR.N = math::isNegative(result);
        APSR.Z = result == 0;
        APSR.C = carry;
    }
}

template <Encoding encoding, typename T>
void cmdMovImmediate(T opCode, CpuRegisterSet& registers)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2, Encoding::T3>);

    if (!registers.conditionPassed()) {
        return;
    }

    auto& APSR = registers.APSR();

    uint8_t d, setFlags;
    uint32_t imm32;
    bool carry;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [imm8, Rd] = math::split<T, Part<0, 8>, Part<8, 3>>(opCode);

        d = Rd;
        setFlags = !registers.isInItBlock();
        imm32 = static_cast<uint32_t>(imm8);
        carry = APSR.C;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [imm8, Rd, imm3, S, i] = math::split<T, Part<0, 8>, Part<8, 4>, Part<12, 3>, Part<20, 1>, Part<26, 1>>(opCode);
        assert(Rd < 13);

        d = Rd;
        setFlags = S;
        std::tie(imm32, carry) =
            math::thumbExpandImmediateWithCarry(math::combine<uint16_t>(Part<0, 8>{imm8}, Part<8, 3>{imm3}, Part<12, 1>{i}), APSR.C);
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>) {
        const auto [imm8, Rd, imm3, imm4, i] = math::split<T, Part<0, 8>, Part<8, 4>, Part<12, 3>, Part<16, 4>, Part<26, 1>>(opCode);

        d = Rd;
        setFlags = false;
        imm32 = math::combine<uint32_t>(Part<0, 8>{imm8}, Part<8, 3>{imm3}, Part<12, 1>{i}, Part<13, 4>{imm4});
        carry = APSR.C;
    }

    auto& Rd = registers.reg(d);

    Rd = imm32;
    if (setFlags) {
        APSR.N = math::isNegative(imm32);
        APSR.Z = imm32 == 0;
        APSR.C = carry;
    }
}

template <Encoding encoding, typename T>
void cmdMovRegister(T opCode, CpuRegisterSet& registers)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2, Encoding::T3>);

    if (!registers.conditionPassed()) {
        return;
    }

    auto& APSR = registers.APSR();

    uint8_t d, m, setFlags;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rd, Rm, D] = math::split<T, Part<0, 3>, Part<3, 4>, Part<7, 1>>(opCode);

        d = math::combine<uint8_t>(Part<0, 3>{Rd}, Part<3, 1>{D});
        assert(d != 15 || !registers.isInItBlock() || registers.isLastInItBlock());

        m = Rm;
        setFlags = false;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint16_t, T>) {
        const auto [Rd, Rm] = math::split<T, Part<0, 3>, Part<3, 3>>(opCode);

        assert(!registers.isInItBlock());

        d = Rd;
        m = Rm;
        setFlags = false;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T3, encoding, uint16_t, T>) {
        const auto [Rm, Rd, S] = math::split<T, Part<0, 4>, Part<8, 4>, Part<20, 1>>(opCode);

        d = Rd;
        m = Rm;
        setFlags = S;

        if (setFlags) {
            UNPREDICTABLE_IF(d >= 13 || m >= 13);
        }
        else {
            UNPREDICTABLE_IF(d == 15 || m == 15 || (d == 13 && m == 13));
        }
    }

    auto& Rd = registers.reg(d);

    const auto result = registers.reg(m);

    if (d == 15) {
        registers.aluWritePC(result);
    }
    else {
        Rd = result;
        if (setFlags) {
            APSR.N = math::isNegative(result);
            APSR.Z = result == 0;
        }
    }
}

template <Encoding encoding, typename T>
void cmdCmpImmediate(T opCode, CpuRegisterSet& registers)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    if (!registers.conditionPassed()) {
        return;
    }

    auto& APSR = registers.APSR();

    uint8_t n;
    uint32_t imm32;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [imm8, Rn] = math::split<T, Part<0, 8>, Part<8, 3>>(opCode);

        n = Rn;
        imm32 = static_cast<uint32_t>(imm8);
    }
    if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [imm8, imm3, Rn, i] = math::split<T, Part<0, 8>, Part<12, 3>, Part<16, 4>, Part<26, 1>>(opCode);
        assert(Rn != 15);

        n = Rn;
        imm32 =
            math::thumbExpandImmediateWithCarry(math::combine<uint32_t>(Part<0, 8>{imm8}, Part<8, 3>{imm3}, Part<12, 1>{i}), APSR.C).first;
    }

    const auto [result, carry, overflow] = math::addWithCarry(registers.reg(n), ~imm32, true);

    APSR.N = math::isNegative(result);
    APSR.Z = result == 0;
    APSR.C = carry;
    APSR.V = overflow;
}

template <Encoding encoding, bool isNegative, typename T>
void cmdCmpRegister(T opCode, CpuRegisterSet& registers)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2> || (check<!isNegative> && encoding == Encoding::T3));

    if (!registers.conditionPassed()) {
        return;
    }

    auto& APSR = registers.APSR();

    uint8_t n;
    uint32_t shifted;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rn, Rm] = math::split<T, Part<0, 3>, Part<3, 3>>(opCode);
        n = Rn;
        shifted = registers.reg(Rm);
    }
    else if constexpr (check<!isNegative> && is_valid_opcode_encoding<Encoding::T2, encoding, uint16_t, T>) {
        const auto [Rn, Rm, N] = math::split<T, Part<0, 3>, Part<3, 4>, Part<7, 1>>(opCode);

        n = math::combine<uint8_t>(Part<0, 3>{Rn}, Part<4, 1>{N});
        assert(n >= 8 || Rm >= 8);
        assert(n != 15 && Rm != 15);

        shifted = registers.reg(Rm);
    }
    else if constexpr ((check<!isNegative> && is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>) ||
                       (check<isNegative> && is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>)) {
        const auto [Rm, type, imm2, imm3, Rn] = math::split<T, Part<0, 4>, Part<4, 2>, Part<6, 2>, Part<12, 3>, Part<16, 4>>(opCode);

        const auto [shiftType, shiftN] = math::decodeImmediateShift(type, math::combine<uint8_t>(Part<0, 2>{imm2}, Part<2, 3>{imm3}));

        n = Rn;
        assert(n != 15 && Rm < 13);

        shifted = math::shift(registers.reg(Rm), shiftType, shiftN, APSR.C);
    }

    const auto& Rn = registers.reg(n);

    if constexpr (check<!isNegative>) {
        shifted = ~shifted;
    }

    const auto [result, carry, overflow] = math::addWithCarry(Rn, shifted, !isNegative);

    APSR.N = math::isNegative(result);
    APSR.Z = result == 0;
    APSR.C = carry;
    APSR.V = overflow;
}

template <Encoding encoding, typename T>
void cmdTstRegister(T opCode, CpuRegisterSet& registers)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    if (!registers.conditionPassed()) {
        return;
    }

    auto& APSR = registers.APSR();

    uint8_t n;
    uint32_t shifted;
    bool carry;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rn, Rm] = math::split<T, Part<0, 3>, Part<3, 3>>(opCode);

        n = Rn;
        shifted = registers.reg(Rm);
        carry = APSR.C;
    }
    if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, type, imm2, imm3, Rn] = math::split<T, Part<0, 4>, Part<4, 2>, Part<6, 2>, Part<12, 3>, Part<16, 4>>(opCode);
        assert(Rn < 13 && Rm < 13);

        const auto [shiftType, shiftN] = math::decodeImmediateShift(type, math::combine<uint8_t>(Part<0, 2>{imm2}, Part<2, 3>{imm3}));

        n = Rn;
        std::tie(shifted, carry) = math::shiftWithCarry(registers.reg(Rm), shiftType, shiftN, APSR.C);
    }

    const auto result = registers.reg(n) & shifted;

    APSR.N = math::isNegative(result);
    APSR.Z = result == 0;
    APSR.C = carry;
}

template <bool withLink>
void cmdBranchAndExecuteRegister(uint16_t opCode, CpuRegisterSet& registers)
{
    if (!registers.conditionPassed()) {
        return;
    }

    const auto m = math::getPart<3, 4>(opCode);
    if constexpr (check<withLink>) {
        UNPREDICTABLE_IF(m == 15);
    }

    const auto& Rm = registers.reg(m);

    if constexpr (check<withLink>) {
        const auto nextInstruction = registers.reg(RegisterType::PC) - 2;
        registers.reg(RegisterType::LR) = nextInstruction | 0b1u;
        registers.blxWritePC(Rm);
    }
    else {
        registers.bxWritePC(Rm);
    }
}

template <Encoding encoding, typename T>
void cmdLoadRegisterLiteral(T opCode, CpuRegisterSet& registers, Memory& memory)
{
    if (!registers.conditionPassed()) {
        return;
    }

    uint8_t t, add;
    uint32_t imm32;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [imm8, Rt] = math::split<T, Part<0, 7>, Part<8, 3>>(opCode);

        t = Rt;
        imm32 = static_cast<uint32_t>(imm8);
        add = true;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [imm12, Rt, U] = math::split<T, Part<0, 12, uint32_t>, Part<12, 4>, Part<23, 1>>(opCode);

        t = Rt;
        imm32 = imm12;
        add = U;
    }

    const auto base = registers.reg(RegisterType::PC) & ~math::ONES<2, uint32_t>;


}

}  // namespace stm32::opcodes
