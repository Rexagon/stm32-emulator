#pragma once

#include "cpu.hpp"
#include "utils/general.hpp"
#include "utils/math.hpp"

namespace stm32::opcodes
{
template <uint8_t offset, uint8_t bitCount, typename T = uint8_t>
using Part = utils::Part<offset, bitCount, T>;

enum class Encoding { T1, T2, T3, T4 };

enum class Bitwise { AND, EOR, ORR, BIC };

template <auto v, auto... vs>
constexpr bool is_in = ((v == vs) || ...);

template <Encoding TargetEnc, Encoding Enc, typename TargetOpCodeType, typename OpCodeType>
constexpr bool is_valid_opcode_encoding = (Enc == TargetEnc) && std::is_same_v<OpCodeType, TargetOpCodeType>;

template <bool b>
constexpr bool check = b;  // Clion unused code highlighting fix

template <Encoding encoding, utils::ShiftType shiftType, typename T>
inline void cmdShiftImmediate(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);
    static_assert(is_in<shiftType, utils::ShiftType::LSL, utils::ShiftType::LSR, utils::ShiftType::ASR>);

    if (!cpu.conditionPassed()) {
        return;
    }

    auto& APSR = cpu.registers().APSR();

    uint8_t d, m, setFlags, shiftN;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rd, Rm, imm5, shiftTypeBits] = utils::split<T, Part<0, 3>, Part<3, 3>, Part<6, 5>, Part<11, 2>>(opCode);

        d = Rd;
        m = Rm;
        setFlags = !cpu.isInItBlock();
        shiftN = utils::decodeImmediateShift(shiftTypeBits, imm5).second;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, imm2, Rd, imm3, S, shiftTypeBits] =
            utils::split<T, Part<0, 4>, Part<6, 2>, Part<8, 4>, Part<12, 3>, Part<20, 1>>(opCode);

        assert(Rd < 13 && Rm < 13);

        d = Rd;
        m = Rm;
        setFlags = S;
        shiftN = utils::decodeImmediateShift(shiftTypeBits, utils::combine<uint8_t>(Part<0, 2>{imm2}, Part<2, 3>{imm3})).second;
    }

    const auto [result, carry] = utils::shiftWithCarry(cpu.R(m), shiftType, shiftN, APSR.C);
    cpu.setR(d, result);

    if (setFlags) {
        APSR.N = utils::isNegative(result);
        APSR.Z = result == 0;
        APSR.C = carry;
    }
}

template <Encoding encoding, utils::ShiftType shiftType, typename T>
inline void cmdShiftRegister(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);
    static_assert(is_in<shiftType, utils::ShiftType::LSL, utils::ShiftType::LSR, utils::ShiftType::ASR, utils::ShiftType::ROR>);

    if (!cpu.conditionPassed()) {
        return;
    }

    auto& APSR = cpu.registers().APSR();

    uint8_t d, n, m, setFlags;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rdn, Rm] = utils::split<T, Part<0, 3>, Part<3, 3>>(opCode);

        d = Rdn;
        n = Rdn;
        m = Rm;
        setFlags = !cpu.isInItBlock();
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, Rd, Rn, S] = utils::split<T, Part<0, 4>, Part<8, 4>, Part<16, 4>, Part<20, 1>>(opCode);

        assert(Rd < 13 && Rn < 13 && Rm < 13);

        d = Rd;
        n = Rn;
        m = Rm;
        setFlags = S;
    }

    const auto shiftN = utils::getPart<0, 8>(cpu.R(m));

    const auto [result, carry] = utils::shiftWithCarry(cpu.R(n), shiftType, shiftN, APSR.C);
    cpu.setR(d, result);

    if (setFlags) {
        APSR.N = utils::isNegative(result);
        APSR.Z = result == 0;
        APSR.C = carry;
    }
}

template <Encoding encoding, bool isSub, typename T>
void cmdAddSubImmediate(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2, Encoding::T3, Encoding::T4>);

    if (!cpu.conditionPassed()) {
        return;
    }

    auto& APSR = cpu.registers().APSR();

    uint8_t d, n, setFlags;
    uint32_t imm32;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rd, Rn, imm3] = utils::split<T, Part<0, 3>, Part<3, 3>, Part<6, 3, uint32_t>>(opCode);

        d = Rd;
        n = Rn;
        setFlags = !cpu.isInItBlock();
        imm32 = imm3;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint16_t, T>) {
        const auto [imm8, Rdn] = utils::split<T, Part<0, 8, uint32_t>, Part<8, 3>>(opCode);

        d = Rdn;
        n = Rdn;
        setFlags = !cpu.isInItBlock();
        imm32 = imm8;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>) {
        const auto [imm8, Rd, imm3, Rn, S, i] =
            utils::split<T, Part<0, 8>, Part<8, 4>, Part<12, 3>, Part<16, 4>, Part<20, 1>, Part<26, 1>>(opCode);

        d = Rd;
        n = Rn;
        setFlags = S;

        const auto combined = utils::combine<uint16_t>(Part<0, 8>{imm8}, Part<8, 3>{imm3}, Part<12, 1>{i});
        imm32 = utils::thumbExpandImmediateWithCarry(combined, APSR.C).first;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T4, encoding, uint32_t, T>) {
        const auto [imm8, Rd, imm3, Rn, i] = utils::split<T, Part<0, 8>, Part<8, 4>, Part<12, 3>, Part<16, 4>, Part<26, 1>>(opCode);

        d = Rd;
        n = Rn;
        setFlags = 0;
        imm32 = utils::combine<uint32_t>(Part<0, 8>{imm8}, Part<8, 3>{imm3}, Part<12, 1>{i});
    }

    if constexpr (check<isSub>) {
        imm32 = ~imm32;
    }

    const auto [result, carry, overflow] = utils::addWithCarry(cpu.R(n), imm32, isSub);
    cpu.setR(d, result);

    if (setFlags) {
        APSR.N = utils::isNegative(result);
        APSR.Z = result == 0;
        APSR.C = carry;
        APSR.V = overflow;
    }
}

template <Encoding encoding, bool isSub, typename T>
void cmdAddSubRegister(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2> || (encoding == Encoding::T3 && !isSub));

    if (!cpu.conditionPassed()) {
        return;
    }

    auto& APSR = cpu.registers().APSR();

    uint8_t d, n, setFlags;
    uint32_t shifted;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rd, Rn, Rm] = utils::split<T, Part<0, 3>, Part<3, 3>, Part<6, 3>>(opCode);

        d = Rd;
        n = Rn;
        setFlags = !cpu.isInItBlock();
        shifted = cpu.R(Rm);
    }
    else if constexpr (check<!isSub> && is_valid_opcode_encoding<Encoding::T2, encoding, uint16_t, T>) {
        const auto [Rdn, Rm, DN] = utils::split<T, Part<0, 3>, Part<3, 4>, Part<7, 1>>(opCode);

        d = utils::combine<uint8_t>(Part<0, 3>{Rdn}, Part<3, 1>{DN});

        UNPREDICTABLE_IF(d == 15 && cpu.isInItBlock() && !cpu.isLastInItBlock());
        UNPREDICTABLE_IF(d == 15 || Rm == 15);

        n = d;
        setFlags = false;
        shifted = cpu.R(Rm);
    }
    else if constexpr ((check<isSub> && is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>) ||
                       (check<!isSub> && is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>)) {
        const auto [Rm, type, imm2, Rd, imm3, Rn, S] =
            utils::split<T, Part<0, 4>, Part<4, 2>, Part<6, 2>, Part<8, 4>, Part<12, 3>, Part<16, 4>, Part<20, 1>>(opCode);

        assert(Rd != 13 && (Rd != 15 || S != 0) && Rn != 15 && Rm < 13);

        const auto [shiftType, shiftN] = utils::decodeImmediateShift(type, utils::combine<uint8_t>(Part<0, 2>{imm2}, Part<2, 3>{imm3}));

        d = Rd;
        n = Rn;
        setFlags = S;
        shifted = utils::shift(cpu.R(Rm), shiftType, shiftN, APSR.C);
    }

    if constexpr (check<isSub>) {
        shifted = ~shifted;
    }

    const auto [result, carry, overflow] = utils::addWithCarry(cpu.R(n), shifted, isSub);

    if (check<!isSub> && d == 15) {
        cpu.aluWritePC(result);
    }
    else {
        cpu.setR(d, result);
        if (setFlags) {
            APSR.N = utils::isNegative(result);
            APSR.Z = result == 0;
            APSR.C = carry;
            APSR.V = overflow;
        }
    }
}

template <Encoding encoding, bool isSub, typename T>
void cmdAddSubSpPlusImmediate(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2, Encoding::T3> || (encoding == Encoding::T4 && !isSub));

    if (!cpu.conditionPassed()) {
        return;
    }

    auto& APSR = cpu.registers().APSR();

    uint8_t d, setFlags;
    uint32_t imm32;
    if constexpr (check<!isSub> && is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [imm8, Rd] = utils::split<T, Part<0, 8, uint32_t>, Part<8, 3>>(opCode);

        d = Rd;
        setFlags = false;
        imm32 = imm8 << 2u;
    }
    else if constexpr ((check<isSub> && is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) ||
                       (check<!isSub> && is_valid_opcode_encoding<Encoding::T2, encoding, uint16_t, T>)) {
        const auto imm7 = utils::getPart<0, 7, uint32_t>(opCode);

        d = rg::RegisterType::SP;
        setFlags = false;
        imm32 = imm7 << 2u;
    }
    else if constexpr ((check<isSub> && is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) ||
                       (check<!isSub> && is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>)) {
        const auto [imm8, Rd, imm3, S, i] = utils::split<T, Part<0, 8>, Part<8, 4>, Part<12, 3>, Part<20, 1>, Part<26, 1>>(opCode);

        d = Rd;
        setFlags = S;
        imm32 = utils::thumbExpandImmediateWithCarry(utils::combine<uint16_t>(Part<0, 8>{imm8}, Part<8, 3>{imm3}, Part<11, 1>{i}), APSR.C)
                    .first;

        UNPREDICTABLE_IF(d == 15 && S == 0);
    }
    else if constexpr ((check<isSub> && is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>) ||
                       (check<!isSub> && is_valid_opcode_encoding<Encoding::T4, encoding, uint32_t, T>)) {
        const auto [imm8, Rd, imm3, i] = utils::split<T, Part<0, 8>, Part<8, 4>, Part<12, 3>, Part<26, 1>>(opCode);

        d = Rd;
        setFlags = false;
        imm32 = utils::combine<uint32_t>(Part<0, 8>{imm8}, Part<8, 3>{imm3}, Part<11, 1>{i});

        UNPREDICTABLE_IF(d == 15);
    }

    if constexpr (isSub) {
        imm32 = ~imm32;
    }

    const auto [result, carry, overflow] = utils::addWithCarry(cpu.registers().SP(), imm32, isSub);
    cpu.setR(d, result);

    if (setFlags) {
        APSR.N = utils::isNegative(result);
        APSR.Z = result == 0u;
        APSR.C = carry;
        APSR.V = overflow;
    }
}

template <Encoding encoding, bool isSbc, typename T>
void cmdAdcSbcRegister(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    if (!cpu.conditionPassed()) {
        return;
    }

    auto& APSR = cpu.registers().APSR();

    uint8_t d, n, setFlags;
    uint32_t shifted;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rdn, Rm] = utils::split<T, Part<0, 3>, Part<3, 3>>(opCode);

        d = Rdn;
        n = Rdn;
        setFlags = !cpu.isInItBlock();
        shifted = cpu.R(Rm);
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, type, imm2, Rd, imm3, Rn, S] =
            utils::split<T, Part<0, 4>, Part<4, 2>, Part<8, 4>, Part<12, 3>, Part<16, 4>, Part<20, 1>>(opCode);

        assert(Rd < 13 && Rn < 13 && Rm < 13);

        const auto [shiftType, shiftN] = utils::decodeImmediateShift(type, utils::combine<uint8_t>(Part<0, 2>{imm2}, Part<2, 3>{imm3}));

        d = Rd;
        n = Rn;
        shifted = utils::shift(cpu.R(Rm), shiftType, shiftN, APSR.C);
    }

    if constexpr (check<isSbc>) {
        shifted = ~shifted;
    }

    const auto [result, carry, overflow] = utils::addWithCarry(cpu.R(n), shifted, isSbc);
    cpu.setR(d, result);

    if (setFlags) {
        APSR.N = utils::isNegative(result);
        APSR.Z = result == 0;
        APSR.C = carry;
        APSR.V = overflow;
    }
}

template <Encoding encoding, typename T>
void cmdRsbImmediate(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    if (!cpu.conditionPassed()) {
        return;
    }

    auto& APSR = cpu.registers().APSR();

    uint8_t d, n, setFlags;
    uint32_t imm32;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rd, Rn] = utils::split<T, Part<0, 3>, Part<3, 3>>(opCode);

        d = Rd;
        n = Rn;
        setFlags = !cpu.isInItBlock();
        imm32 = 0x0u;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [imm8, Rd, imm3, Rn, S, i] =
            utils::split<T, Part<0, 8>, Part<8, 4>, Part<12, 3>, Part<16, 4>, Part<20, 1>, Part<26, 1>>(opCode);

        assert(Rd < 13 && Rn < 13);

        d = Rd;
        n = Rn;
        setFlags = S;
        imm32 = utils::thumbExpandImmediateWithCarry(utils::combine<uint32_t>(Part<0, 8>{imm8}, Part<8, 3>{imm3}, Part<12, 1>{i}), APSR.C)
                    .first;
    }

    const auto [result, carry, overflow] = utils::addWithCarry(~cpu.R(n), imm32, true);
    cpu.setR(d, result);

    if (setFlags) {
        APSR.N = utils::isNegative(result);
        APSR.Z = result == 0;
        APSR.C = carry;
        APSR.V = overflow;
    }
}

template <Encoding encoding, typename T>
void cmdMul(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    if (!cpu.conditionPassed()) {
        return;
    }

    auto& APSR = cpu.registers().APSR();

    uint8_t d, n, m, setFlags;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rdm, Rn] = utils::split<T, Part<0, 3>, Part<3, 3>>(opCode);

        d = Rdm;
        n = Rn;
        m = Rdm;
        setFlags = !cpu.isInItBlock();
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, Rd, Rn] = utils::split<T, Part<0, 4>, Part<8, 4>, Part<16, 4>>(opCode);
        assert(Rd < 13 && n < 13 && m < 13);

        d = Rd;
        n = Rn;
        m = Rm;
        setFlags = false;
    }

    const auto result = cpu.R(n) * cpu.R(m);
    cpu.setR(d, result);

    if (setFlags) {
        APSR.N = utils::isNegative(result);
        APSR.Z = result == 0;
    }
}

template <Encoding encoding, Bitwise bitwise, typename T>
void cmdBitwiseRegister(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    if (!cpu.conditionPassed()) {
        return;
    }

    auto& APSR = cpu.registers().APSR();

    uint8_t d, n, setFlags;
    uint32_t shifted;
    bool carry;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rdn, Rm] = utils::split<T, Part<0, 3>, Part<3, 3>>(opCode);

        d = Rdn;
        n = Rdn;
        setFlags = !cpu.isInItBlock();

        shifted = cpu.R(Rm);
        carry = APSR.C;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, type, imm2, Rd, imm3, Rn, S] =
            utils::split<T, Part<0, 4>, Part<4, 2>, Part<6, 2>, Part<8, 4>, Part<12, 3>, Part<16, 4>, Part<20, 1>>(opCode);

        const auto [shiftType, shiftN] = utils::decodeImmediateShift(type, utils::combine<uint8_t>(Part<0, 2>{imm2}, Part<2, 3>{imm3}));

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

        std::tie(shifted, carry) = utils::shiftWithCarry(cpu.R(Rm), shiftType, shiftN, APSR.C);
    }

    uint32_t result;
    if constexpr (is_in<bitwise, Bitwise::AND>) {
        result = cpu.R(n) & shifted;
    }
    else if constexpr (is_in<bitwise, Bitwise::EOR>) {
        result = cpu.R(n) ^ shifted;
    }
    else if constexpr (is_in<bitwise, Bitwise::ORR>) {
        result = cpu.R(n) | shifted;
    }
    else if constexpr (is_in<bitwise, Bitwise::BIC>) {
        result = cpu.R(n) & ~shifted;
    }
    cpu.setR(d, result);

    if (setFlags) {
        APSR.N = utils::isNegative(result);
        APSR.Z = result == 0;
        APSR.C = carry;
    }
}

template <Encoding encoding, typename T>
void cmdMvnRegister(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    if (!cpu.conditionPassed()) {
        return;
    }

    auto& APSR = cpu.registers().APSR();

    uint8_t d, setFlags;
    uint32_t shifted;
    bool carry;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rd, Rm] = utils::split<T, Part<0, 3>, Part<3, 3>>(opCode);

        d = Rd;
        setFlags = !cpu.isInItBlock();

        shifted = cpu.R(Rm);
        carry = APSR.C;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, type, imm2, Rd, imm3, S] =
            utils::split<T, Part<0, 4>, Part<4, 2>, Part<6, 2>, Part<8, 4>, Part<12, 3>, Part<20, 1>>(opCode);

        assert(Rd < 13 && Rm < 13);

        const auto [shiftType, shiftN] = utils::decodeImmediateShift(type, utils::combine<uint8_t>(Part<0, 2>{imm2}, Part<2, 3>{imm3}));

        d = Rd;
        setFlags = S;

        std::tie(shifted, carry) = utils::shiftWithCarry(cpu.R(Rm), shiftType, shiftN, APSR.C);
    }

    const auto result = ~shifted;
    cpu.setR(d, result);

    if (setFlags) {
        APSR.N = utils::isNegative(result);
        APSR.Z = result == 0;
        APSR.C = carry;
    }
}

template <Encoding encoding, typename T>
void cmdMovImmediate(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2, Encoding::T3>);

    if (!cpu.conditionPassed()) {
        return;
    }

    auto& APSR = cpu.registers().APSR();

    uint8_t d, setFlags;
    uint32_t imm32;
    bool carry;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [imm8, Rd] = utils::split<T, Part<0, 8>, Part<8, 3>>(opCode);

        d = Rd;
        setFlags = !cpu.isInItBlock();
        imm32 = static_cast<uint32_t>(imm8);
        carry = APSR.C;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [imm8, Rd, imm3, S, i] = utils::split<T, Part<0, 8>, Part<8, 4>, Part<12, 3>, Part<20, 1>, Part<26, 1>>(opCode);
        assert(Rd < 13);

        d = Rd;
        setFlags = S;
        std::tie(imm32, carry) =
            utils::thumbExpandImmediateWithCarry(utils::combine<uint16_t>(Part<0, 8>{imm8}, Part<8, 3>{imm3}, Part<12, 1>{i}), APSR.C);
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>) {
        const auto [imm8, Rd, imm3, imm4, i] = utils::split<T, Part<0, 8>, Part<8, 4>, Part<12, 3>, Part<16, 4>, Part<26, 1>>(opCode);

        d = Rd;
        setFlags = false;
        imm32 = utils::combine<uint32_t>(Part<0, 8>{imm8}, Part<8, 3>{imm3}, Part<12, 1>{i}, Part<13, 4>{imm4});
        carry = APSR.C;
    }

    cpu.setR(d, imm32);

    if (setFlags) {
        APSR.N = utils::isNegative(imm32);
        APSR.Z = imm32 == 0;
        APSR.C = carry;
    }
}

template <Encoding encoding, typename T>
void cmdMovRegister(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2, Encoding::T3>);

    if (!cpu.conditionPassed()) {
        return;
    }

    auto& APSR = cpu.registers().APSR();

    uint8_t d, m, setFlags;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rd, Rm, D] = utils::split<T, Part<0, 3>, Part<3, 4>, Part<7, 1>>(opCode);

        d = utils::combine<uint8_t>(Part<0, 3>{Rd}, Part<3, 1>{D});
        assert(d != 15 || !cpu.isInItBlock() || cpu.isLastInItBlock());

        m = Rm;
        setFlags = false;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint16_t, T>) {
        const auto [Rd, Rm] = utils::split<T, Part<0, 3>, Part<3, 3>>(opCode);

        assert(!cpu.isInItBlock());

        d = Rd;
        m = Rm;
        setFlags = false;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T3, encoding, uint16_t, T>) {
        const auto [Rm, Rd, S] = utils::split<T, Part<0, 4>, Part<8, 4>, Part<20, 1>>(opCode);

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

    const auto result = cpu.R(m);

    if (d == 15) {
        cpu.aluWritePC(result);
    }
    else {
        cpu.setR(d, result);
        if (setFlags) {
            APSR.N = utils::isNegative(result);
            APSR.Z = result == 0;
        }
    }
}

template <Encoding encoding, typename T>
void cmdCmpImmediate(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    if (!cpu.conditionPassed()) {
        return;
    }

    auto& APSR = cpu.registers().APSR();

    uint8_t n;
    uint32_t imm32;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [imm8, Rn] = utils::split<T, Part<0, 8>, Part<8, 3>>(opCode);

        n = Rn;
        imm32 = static_cast<uint32_t>(imm8);
    }
    if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [imm8, imm3, Rn, i] = utils::split<T, Part<0, 8>, Part<12, 3>, Part<16, 4>, Part<26, 1>>(opCode);
        assert(Rn != 15);

        n = Rn;
        imm32 = utils::thumbExpandImmediateWithCarry(utils::combine<uint32_t>(Part<0, 8>{imm8}, Part<8, 3>{imm3}, Part<12, 1>{i}), APSR.C)
                    .first;
    }

    const auto [result, carry, overflow] = utils::addWithCarry(cpu.R(n), ~imm32, true);

    APSR.N = utils::isNegative(result);
    APSR.Z = result == 0;
    APSR.C = carry;
    APSR.V = overflow;
}

template <Encoding encoding, bool isNegative, typename T>
void cmdCmpRegister(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2> || (check<!isNegative> && encoding == Encoding::T3));

    if (!cpu.conditionPassed()) {
        return;
    }

    auto& APSR = cpu.registers().APSR();

    uint8_t n;
    uint32_t shifted;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rn, Rm] = utils::split<T, Part<0, 3>, Part<3, 3>>(opCode);
        n = Rn;
        shifted = cpu.R(Rm);
    }
    else if constexpr (check<!isNegative> && is_valid_opcode_encoding<Encoding::T2, encoding, uint16_t, T>) {
        const auto [Rn, Rm, N] = utils::split<T, Part<0, 3>, Part<3, 4>, Part<7, 1>>(opCode);

        n = utils::combine<uint8_t>(Part<0, 3>{Rn}, Part<4, 1>{N});
        assert(n >= 8 || Rm >= 8);
        assert(n != 15 && Rm != 15);

        shifted = cpu.R(Rm);
    }
    else if constexpr ((check<!isNegative> && is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>) ||
                       (check<isNegative> && is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>)) {
        const auto [Rm, type, imm2, imm3, Rn] = utils::split<T, Part<0, 4>, Part<4, 2>, Part<6, 2>, Part<12, 3>, Part<16, 4>>(opCode);

        const auto [shiftType, shiftN] = utils::decodeImmediateShift(type, utils::combine<uint8_t>(Part<0, 2>{imm2}, Part<2, 3>{imm3}));

        n = Rn;
        assert(n != 15 && Rm < 13);

        shifted = utils::shift(cpu.R(Rm), shiftType, shiftN, APSR.C);
    }

    if constexpr (check<!isNegative>) {
        shifted = ~shifted;
    }

    const auto [result, carry, overflow] = utils::addWithCarry(cpu.R(n), shifted, !isNegative);

    APSR.N = utils::isNegative(result);
    APSR.Z = result == 0;
    APSR.C = carry;
    APSR.V = overflow;
}

template <Encoding encoding, typename T>
void cmdTstRegister(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    if (!cpu.conditionPassed()) {
        return;
    }

    auto& APSR = cpu.registers().APSR();

    uint8_t n;
    uint32_t shifted;
    bool carry;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rn, Rm] = utils::split<T, Part<0, 3>, Part<3, 3>>(opCode);

        n = Rn;
        shifted = cpu.R(Rm);
        carry = APSR.C;
    }
    if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, type, imm2, imm3, Rn] = utils::split<T, Part<0, 4>, Part<4, 2>, Part<6, 2>, Part<12, 3>, Part<16, 4>>(opCode);
        assert(Rn < 13 && Rm < 13);

        const auto [shiftType, shiftN] = utils::decodeImmediateShift(type, utils::combine<uint8_t>(Part<0, 2>{imm2}, Part<2, 3>{imm3}));

        n = Rn;
        std::tie(shifted, carry) = utils::shiftWithCarry(cpu.R(Rm), shiftType, shiftN, APSR.C);
    }

    const auto result = cpu.R(n) & shifted;

    APSR.N = utils::isNegative(result);
    APSR.Z = result == 0;
    APSR.C = carry;
}

template <bool withLink>
void cmdBranchAndExecuteRegister(uint16_t opCode, Cpu& cpu)
{
    if (!cpu.conditionPassed()) {
        return;
    }

    const auto m = utils::getPart<3, 4>(opCode);
    if constexpr (check<withLink>) {
        UNPREDICTABLE_IF(m == 15);
    }
    UNPREDICTABLE_IF(cpu.isInItBlock() && !cpu.isLastInItBlock());

    if constexpr (check<withLink>) {
        const auto nextInstruction = cpu.currentInstructionAddress() + 2u;
        cpu.registers().LR() = nextInstruction | 0b1u;
        cpu.blxWritePC(cpu.R(m));
    }
    else {
        cpu.bxWritePC(cpu.R(m));
    }
}

template <Encoding encoding, typename T>
void cmdBranch(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2, Encoding::T3, Encoding::T4>);

    if (!cpu.conditionPassed()) {
        return;
    }

    uint32_t imm32;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [imm8, cond] = utils::split<T, Part<0, 8, uint32_t>, Part<8, 4>>(opCode);

        imm32 = utils::signExtend<9>(imm8 << 1);
        UNPREDICTABLE_IF(cpu.isInItBlock());
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint16_t, T>) {
        const auto imm11 = utils::getPart<0, 11, uint32_t>(opCode);

        imm32 = utils::signExtend<12>(imm11 << 1);
        UNPREDICTABLE_IF(cpu.isInItBlock() && !cpu.isLastInItBlock());
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>) {
        const auto [imm11, J2, J1, imm6, cond, S] =
            utils::getPart<T, Part<0, 11>, Part<11, 1>, Part<13, 1>, Part<16, 6>, Part<22, 4>, Part<26, 1>>(opCode);

        imm32 = utils::signExtend<21>(
            utils::combine<T>(Part<0, 1>{0u}, Part<1, 11>{imm11}, Part<12, 6>{imm6}, Part<18, 1>{J1}, Part<19, 1>{J2}, Part<20, 1>{S}));

        UNPREDICTABLE_IF(cpu.isInItBlock());
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T4, encoding, uint32_t, T>) {
        const auto [imm11, J2, J1, imm10, S] = utils::getPart<T, Part<0, 11>, Part<11, 1>, Part<13, 1>, Part<16, 10>, Part<26, 1>>(opCode);

        const auto I1 = ~(J1 ^ S);
        const auto I2 = ~(J2 ^ S);

        imm32 = utils::signExtend<25>(
            utils::combine<T>(Part<0, 1>{0u}, Part<1, 11>{imm11}, Part<12, 10>{imm10}, Part<22, 1>{I2}, Part<23, 1>{I1}, Part<24, 1>{S}));

        UNPREDICTABLE_IF(cpu.isInItBlock() && !cpu.isLastInItBlock());
    }

    cpu.branchWritePC(cpu.currentInstructionAddress() + 4u + imm32);
}

template <Encoding encoding, typename T>
void cmdPermanentlyUndefined(T /*opCode*/, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    if (!cpu.conditionPassed()) {
        return;
    }

    // TODO: raise UNDEFINED
}


inline void cmdCallSupervisor(uint16_t /*opCode*/, Cpu& cpu)
{
    if (!cpu.conditionPassed()) {
        return;
    }

    // TODO: call supervisor
}

template <Encoding encoding, typename T>
void cmdAdr(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2, Encoding::T3>);

    if (!cpu.conditionPassed()) {
        return;
    }

    uint8_t d, add;
    uint32_t imm32;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [imm8, Rd] = utils::split<T, Part<0, 8, uint32_t>, Part<8, 3>>(opCode);

        d = Rd;
        imm32 = imm8;
        add = true;

        UNPREDICTABLE_IF(d >= 13);
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T> ||
                       is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>) {
        const auto [imm8, Rd, imm3, S, i] = utils::split<T, Part<0, 8>, Part<8, 4>, Part<12, 3>, Part<23, 1>, Part<26, 1>>(opCode);

        d = Rd;
        imm32 = utils::combine<T>(Part<0, 8>{imm8}, Part<8, 3>{imm3}, Part<11, 1>{i});
        add = S == 0u;

        UNPREDICTABLE_IF(d >= 13);
    }

    const auto PC = (cpu.currentInstructionAddress() + 4u) & utils::ZEROS<4, uint32_t>;

    uint32_t result;
    if (add) {
        result = PC + imm32;
    }
    else {
        result = PC - imm32;
    }

    cpu.setR(d, result);
}

template <Encoding encoding, typename T>
void cmdLoadRegisterLiteral(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    if (!cpu.conditionPassed()) {
        return;
    }

    uint8_t t, add;
    uint32_t imm32;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [imm8, Rt] = utils::split<T, Part<0, 7>, Part<8, 3>>(opCode);

        t = Rt;
        imm32 = utils::combine<uint32_t>(Part<0, 2>{0u}, Part<2, 8>{imm8});
        add = true;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [imm12, Rt, U] = utils::split<T, Part<0, 12, uint32_t>, Part<12, 4>, Part<23, 1>>(opCode);

        t = Rt;
        imm32 = imm12;
        add = U;

        UNPREDICTABLE_IF(t == 15 && cpu.isInItBlock() && !cpu.isLastInItBlock());
    }

    const auto base = (cpu.currentInstructionAddress() + 4u) & utils::ZEROS<2, uint32_t>;

    uint32_t address;
    if (add) {
        address = base + imm32;
    }
    else {
        address = base - imm32;
    }

    const auto data = cpu.mpu().unalignedMemoryRead<uint32_t>(address);
    if (t == 15) {
        if ((data & utils::ONES<2, uint32_t>) == 0u) {
            cpu.loadWritePC(data);
        }
        else {
            UNPREDICTABLE;
        }
    }
    else {
        cpu.setR(t, data);
    }
}

inline void cmdCps(uint16_t opCode, Cpu& cpu)
{
    if (!cpu.isInPrivilegedMode()) {
        return;
    }

    const auto [F, I, im] = utils::split<uint16_t, Part<0, 1>, Part<1, 1>, Part<4, 1>>(opCode);

    UNPREDICTABLE_IF(I == 0 && F == 0);

    const auto affectPRI = I == 1u;
    const auto affectFAULT = F == 1u;

    UNPREDICTABLE_IF(cpu.isInItBlock());

    if (im == 0) {
        if (affectPRI) {
            cpu.registers().PRIMASK().PM = false;
        }
        if (affectFAULT) {
            cpu.registers().FAULTMASK().FM = false;
        }
    }
    else {
        if (affectPRI) {
            cpu.registers().PRIMASK().PM = true;
        }
        if (affectFAULT && cpu.executionPriority() > -1) {
            cpu.registers().FAULTMASK().FM = true;
        }
    }
}

}  // namespace stm32::opcodes
