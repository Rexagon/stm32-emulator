#pragma once

#include "cpu.hpp"
#include "utils/general.hpp"
#include "utils/math.hpp"

namespace stm32::opcodes
{
#define CHECK_CONDITION         \
    if (!cpu.conditionPassed()) \
    return

template <uint8_t offset, uint8_t bitCount, typename T = uint8_t>
using _ = utils::Part<offset, bitCount, T>;

enum class Encoding { T1, T2, T3, T4 };

enum class Bitwise { AND, EOR, ORR, BIC };

enum class Hint {
    Nop,
    Yield,
    WaitForEvent,
    WaitForInterrupt,
    SendEvent,
};

enum class Control {
    ClearExclusive,
    DataSynchronizationBarrier,
    DataMemoryBarrier,
    InstructionSynchronizationBarrier,
};

template <auto v, auto... vs>
constexpr bool is_in = ((v == vs) || ...);

template <Encoding TargetEnc, Encoding Enc, typename TargetOpCodeType, typename OpCodeType>
constexpr bool is_valid_opcode_encoding = (Enc == TargetEnc) && std::is_same_v<OpCodeType, TargetOpCodeType>;

template <Encoding encoding, utils::ShiftType shiftType, typename T>
inline void cmdShiftImmediate(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);
    static_assert(is_in<shiftType, utils::ShiftType::LSL, utils::ShiftType::LSR, utils::ShiftType::ASR>);

    CHECK_CONDITION;

    auto& APSR = cpu.registers().APSR();

    uint8_t d, m, setFlags, shiftN;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rd, Rm, imm5, shiftTypeBits] = utils::split<_<0, 3>, _<3, 3>, _<6, 5>, _<11, 2>>(opCode);

        d = Rd;
        m = Rm;
        setFlags = !cpu.isInItBlock();
        shiftN = utils::decodeImmediateShift(shiftTypeBits, imm5).second;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, shiftTypeBits, imm2, Rd, imm3, S] = utils::split<_<0, 4>, _<4, 2>, _<6, 2>, _<8, 4>, _<12, 3>, _<20, 1>>(opCode);

        UNPREDICTABLE_IF(Rd >= 13 || Rm >= 13);

        d = Rd;
        m = Rm;
        setFlags = S;
        shiftN = utils::decodeImmediateShift(shiftTypeBits, utils::combine<uint8_t>(_<0, 2>{imm2}, _<2, 3>{imm3})).second;
    }

    const auto [result, carry] = utils::shiftWithCarry(cpu.R(m), shiftType, shiftN, APSR.C);
    cpu.setR(d, result);

    if (setFlags) {
        APSR.N = utils::isNegative(result);
        APSR.Z = result == 0;
        APSR.C = carry;
    }
}

inline void cmdRorImmediate(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto [Rm, imm2, Rd, imm3, S] = utils::split<_<0, 4>, _<6, 2>, _<8, 4>, _<12, 3>, _<20, 1>>(opCode);
    UNPREDICTABLE_IF(Rd >= 13 || Rm >= 13);

    auto& APSR = cpu.registers().APSR();

    const auto shiftN = utils::decodeImmediateShift(0b11u, utils::combine<uint8_t>(_<0, 2>{imm2}, _<2, 3>{imm3})).second;

    const auto [result, carry] = utils::shiftWithCarry(cpu.R(Rm), utils::ShiftType::ROR, shiftN, APSR.C);
    cpu.setR(Rd, result);

    if (S) {
        APSR.N = utils::isNegative(result);
        APSR.Z = result == 0u;
        APSR.C = carry;
    }
}

inline void cmdOrnRegister(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto [Rm, type, imm2, Rd, imm3, Rn, S] = utils::split<_<0, 4>, _<4, 2>, _<6, 2>, _<8, 4>, _<12, 3>, _<16, 4>, _<20, 1>>(opCode);
    UNPREDICTABLE_IF(Rd >= 13 || Rn == 13 || Rm >= 13);

    auto& APSR = cpu.registers().APSR();

    const auto [shiftType, shiftN] = utils::decodeImmediateShift(type, utils::combine<uint8_t>(_<0, 2>{imm2}, _<2, 3>{imm3}));
    const auto [shifted, carry] = utils::shiftWithCarry(cpu.R(Rm), shiftType, shiftN, APSR.C);

    const auto result = cpu.R(Rn) | ~shifted;
    cpu.setR(Rd, result);

    if (S) {
        APSR.N = utils::isNegative(result);
        APSR.Z = result == 0u;
        APSR.C = carry;
    }
}

inline void cmdOrnImmediate(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto [imm8, Rd, imm3, Rn, S, i] = utils::split<_<0, 8>, _<8, 4>, _<12, 3>, _<16, 4>, _<20, 1>, _<26, 1>>(opCode);
    UNPREDICTABLE_IF(Rd >= 13 || Rn == 13);

    auto& APSR = cpu.registers().APSR();

    const auto [imm32, carry] =
        utils::thumbExpandImmediateWithCarry(utils::combine<uint16_t>(_<0, 8>{imm8}, _<8, 3>{imm3}, _<11, 1>{i}), APSR.C);

    const auto result = cpu.R(Rn) | ~imm32;
    cpu.setR(Rd, result);

    if (S) {
        APSR.N = utils::isNegative(result);
        APSR.Z = result == 0u;
        APSR.C = carry;
    }
}

inline void cmdTeqRegister(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto [Rm, type, imm2, imm3, Rn] = utils::split<_<0, 4>, _<4, 2>, _<6, 2>, _<12, 3>, _<16, 4>>(opCode);
    UNPREDICTABLE_IF(Rn >= 13 || Rm >= 13);

    auto& APSR = cpu.registers().APSR();

    const auto [shiftType, shiftN] = utils::decodeImmediateShift(type, utils::combine<uint8_t>(_<0, 2>{imm2}, _<2, 3>{imm3}));
    const auto [shifted, carry] = utils::shiftWithCarry(cpu.R(Rm), shiftType, shiftN, APSR.C);

    const auto result = cpu.R(Rn) ^ shifted;

    APSR.N = utils::isNegative(result);
    APSR.Z = result == 0u;
    APSR.C = carry;
}

inline void cmdTeqImmediate(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto [imm8, imm3, Rn, i] = utils::split<_<0, 8>, _<12, 3>, _<16, 4>, _<26, 1>>(opCode);
    UNPREDICTABLE_IF(Rn >= 13);

    auto& APSR = cpu.registers().APSR();

    const auto [imm32, carry] =
        utils::thumbExpandImmediateWithCarry(utils::combine<uint16_t>(_<0, 8>{imm8}, _<8, 3>{imm3}, _<11, 1>{i}), APSR.C);

    const auto result = cpu.R(Rn) ^ imm32;

    APSR.N = utils::isNegative(result);
    APSR.Z = result == 0u;
    APSR.C = carry;
}

inline void cmdRsbRegister(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto [Rm, type, imm2, Rd, imm3, Rn, S] = utils::split<_<0, 4>, _<4, 2>, _<6, 2>, _<8, 4>, _<12, 3>, _<16, 4>, _<20, 1>>(opCode);
    UNPREDICTABLE_IF(Rd >= 13 || Rn >= 13 || Rm >= 13);

    auto& APSR = cpu.registers().APSR();

    const auto [shiftType, shiftN] = utils::decodeImmediateShift(type, utils::combine<uint8_t>(_<0, 2>{imm2}, _<2, 3>{imm3}));
    const auto shifted = utils::shift(cpu.R(Rm), shiftType, shiftN, APSR.C);

    const auto [result, carry, overflow] = utils::addWithCarry(~cpu.R(Rn), shifted, true);
    cpu.setR(Rd, result);

    if (S) {
        APSR.N = utils::isNegative(result);
        APSR.Z = result == 0u;
        APSR.C = carry;
        APSR.V = overflow;
    }
}

inline void cmdRrxImmediate(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto [Rm, Rd, S] = utils::split<_<0, 4>, _<8, 4>, _<20, 1>>(opCode);
    UNPREDICTABLE_IF(Rd >= 13 || Rm >= 13);

    auto& APSR = cpu.registers().APSR();

    const auto [result, carry] = utils::shiftWithCarry(cpu.R(Rm), utils::ShiftType::RRX, 1u, APSR.C);
    cpu.setR(Rd, result);

    if (S) {
        APSR.N = utils::isNegative(result);
        APSR.Z = result == 0u;
        APSR.C = carry;
    }
}

template <Encoding encoding, utils::ShiftType shiftType, typename T>
inline void cmdShiftRegister(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);
    static_assert(is_in<shiftType, utils::ShiftType::LSL, utils::ShiftType::LSR, utils::ShiftType::ASR, utils::ShiftType::ROR>);

    CHECK_CONDITION;

    auto& APSR = cpu.registers().APSR();

    uint8_t d, n, m, setFlags;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rdn, Rm] = utils::split<_<0, 3>, _<3, 3>>(opCode);

        d = Rdn;
        n = Rdn;
        m = Rm;
        setFlags = !cpu.isInItBlock();
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, Rd, Rn, S] = utils::split<_<0, 4>, _<8, 4>, _<16, 4>, _<20, 1>>(opCode);

        UNPREDICTABLE_IF(Rd >= 13 || Rn >= 13 || Rm >= 13);

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

    CHECK_CONDITION;

    auto& APSR = cpu.registers().APSR();

    uint8_t d, n, setFlags;
    uint32_t imm32;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rd, Rn, imm3] = utils::split<_<0, 3>, _<3, 3>, _<6, 3, uint32_t>>(opCode);

        d = Rd;
        n = Rn;
        setFlags = !cpu.isInItBlock();
        imm32 = imm3;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint16_t, T>) {
        const auto [imm8, Rdn] = utils::split<_<0, 8, uint32_t>, _<8, 3>>(opCode);

        d = Rdn;
        n = Rdn;
        setFlags = !cpu.isInItBlock();
        imm32 = imm8;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>) {
        const auto [imm8, Rd, imm3, Rn, S, i] = utils::split<_<0, 8>, _<8, 4>, _<12, 3>, _<16, 4>, _<20, 1>, _<26, 1>>(opCode);

        d = Rd;
        n = Rn;
        setFlags = S;

        const auto combined = utils::combine<uint16_t>(_<0, 8>{imm8}, _<8, 3>{imm3}, _<11, 1>{i});
        imm32 = utils::thumbExpandImmediateWithCarry(combined, APSR.C).first;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T4, encoding, uint32_t, T>) {
        const auto [imm8, Rd, imm3, Rn, i] = utils::split<_<0, 8>, _<8, 4>, _<12, 3>, _<16, 4>, _<26, 1>>(opCode);

        d = Rd;
        n = Rn;
        setFlags = 0;
        imm32 = utils::combine<uint32_t>(_<0, 8>{imm8}, _<8, 3>{imm3}, _<11, 1>{i});
    }

    if constexpr (isSub) {
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

    CHECK_CONDITION;

    auto& APSR = cpu.registers().APSR();

    uint8_t d, n, setFlags;
    uint32_t shifted;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rd, Rn, Rm] = utils::split<_<0, 3>, _<3, 3>, _<6, 3>>(opCode);

        d = Rd;
        n = Rn;
        setFlags = !cpu.isInItBlock();
        shifted = cpu.R(Rm);
    }
    else if constexpr (!isSub && is_valid_opcode_encoding<Encoding::T2, encoding, uint16_t, T>) {
        const auto [Rdn, Rm, DN] = utils::split<_<0, 3>, _<3, 4>, _<7, 1>>(opCode);

        d = utils::combine<uint8_t>(_<0, 3>{Rdn}, _<3, 1>{DN});

        UNPREDICTABLE_IF(d == 15 && cpu.isInItBlock() && !cpu.isLastInItBlock());
        UNPREDICTABLE_IF(d == 15 || Rm == 15);

        n = d;
        setFlags = false;
        shifted = cpu.R(Rm);
    }
    else if constexpr ((!isSub && is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>) ||
                       (isSub && is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>)) {
        const auto [Rm, type, imm2, Rd, imm3, Rn, S] =
            utils::split<_<0, 4>, _<4, 2>, _<6, 2>, _<8, 4>, _<12, 3>, _<16, 4>, _<20, 1>>(opCode);

        UNPREDICTABLE_IF(Rd == 13 || (Rd == 15 && S == 0) || Rn == 15 || Rm >= 13);

        const auto [shiftType, shiftN] = utils::decodeImmediateShift(type, utils::combine<uint8_t>(_<0, 2>{imm2}, _<2, 3>{imm3}));

        d = Rd;
        n = Rn;
        setFlags = S;
        shifted = utils::shift(cpu.R(Rm), shiftType, shiftN, APSR.C);
    }

    if constexpr (isSub) {
        shifted = ~shifted;
    }

    const auto [result, carry, overflow] = utils::addWithCarry(cpu.R(n), shifted, isSub);

    if (!isSub && d == 15) {
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

    CHECK_CONDITION;

    auto& APSR = cpu.registers().APSR();

    uint8_t d, setFlags;
    uint32_t imm32;
    if constexpr (!isSub && is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [imm8, Rd] = utils::split<_<0, 8, uint32_t>, _<8, 3>>(opCode);

        d = Rd;
        setFlags = false;
        imm32 = imm8 << 2u;
    }
    else if constexpr ((isSub && is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) ||
                       (!isSub && is_valid_opcode_encoding<Encoding::T2, encoding, uint16_t, T>)) {
        const auto imm7 = utils::getPart<0, 7, uint32_t>(opCode);

        d = rg::RegisterType::SP;
        setFlags = false;
        imm32 = imm7 << 2u;
    }
    else if constexpr ((isSub && is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) ||
                       (!isSub && is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>)) {
        const auto [imm8, Rd, imm3, S, i] = utils::split<_<0, 8>, _<8, 4>, _<12, 3>, _<20, 1>, _<26, 1>>(opCode);

        d = Rd;
        setFlags = S;
        imm32 = utils::thumbExpandImmediateWithCarry(utils::combine<uint16_t>(_<0, 8>{imm8}, _<8, 3>{imm3}, _<11, 1>{i}), APSR.C).first;

        UNPREDICTABLE_IF(d == 15 && S == 0);
    }
    else if constexpr ((isSub && is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>) ||
                       (!isSub && is_valid_opcode_encoding<Encoding::T4, encoding, uint32_t, T>)) {
        const auto [imm8, Rd, imm3, i] = utils::split<_<0, 8>, _<8, 4>, _<12, 3>, _<26, 1>>(opCode);

        d = Rd;
        setFlags = false;
        imm32 = utils::combine<uint32_t>(_<0, 8>{imm8}, _<8, 3>{imm3}, _<11, 1>{i});

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

    CHECK_CONDITION;

    auto& APSR = cpu.registers().APSR();

    uint8_t d, n, setFlags;
    uint32_t shifted;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rdn, Rm] = utils::split<_<0, 3>, _<3, 3>>(opCode);

        d = Rdn;
        n = Rdn;
        setFlags = !cpu.isInItBlock();
        shifted = cpu.R(Rm);
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, type, imm2, Rd, imm3, Rn, S] =
            utils::split<_<0, 4>, _<4, 2>, _<6, 2>, _<8, 4>, _<12, 3>, _<16, 4>, _<20, 1>>(opCode);

        UNPREDICTABLE_IF(Rd >= 13 || Rn >= 13 || Rm >= 13);

        const auto [shiftType, shiftN] = utils::decodeImmediateShift(type, utils::combine<uint8_t>(_<0, 2>{imm2}, _<2, 3>{imm3}));

        d = Rd;
        n = Rn;
        setFlags = S;
        shifted = utils::shift(cpu.R(Rm), shiftType, shiftN, APSR.C);
    }

    if constexpr (isSbc) {
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

template <bool isSbc>
void cmdAdcSbcImmediate(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto [imm8, Rd, imm3, Rn, S, i] = utils::split<_<0, 8>, _<8, 4>, _<12, 3>, _<16, 4>, _<20, 1>, _<26, 1>>(opCode);
    UNPREDICTABLE_IF(Rd >= 13 || Rn >= 13);

    auto& APSR = cpu.registers().APSR();

    auto imm32 = utils::thumbExpandImmediateWithCarry(utils::combine<uint16_t>(_<0, 8>{imm8}, _<8, 3>{imm3}, _<11, 1>{i}), APSR.C).first;

    if constexpr (isSbc) {
        imm32 = ~imm32;
    }

    const auto [result, carry, overflow] = utils::addWithCarry(cpu.R(Rn), imm32, APSR.C);
    cpu.setR(Rd, result);

    if (S) {
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

    CHECK_CONDITION;

    auto& APSR = cpu.registers().APSR();

    uint8_t d, n, setFlags;
    uint32_t imm32;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rd, Rn] = utils::split<_<0, 3>, _<3, 3>>(opCode);

        d = Rd;
        n = Rn;
        setFlags = !cpu.isInItBlock();
        imm32 = 0x0u;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [imm8, Rd, imm3, Rn, S, i] = utils::split<_<0, 8>, _<8, 4>, _<12, 3>, _<16, 4>, _<20, 1>, _<26, 1>>(opCode);

        UNPREDICTABLE_IF(Rd >= 13 || Rn >= 13);

        d = Rd;
        n = Rn;
        setFlags = S;
        imm32 = utils::thumbExpandImmediateWithCarry(utils::combine<uint16_t>(_<0, 8>{imm8}, _<8, 3>{imm3}, _<11, 1>{i}), APSR.C).first;
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

    CHECK_CONDITION;

    auto& APSR = cpu.registers().APSR();

    uint8_t d, n, m, setFlags;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rdm, Rn] = utils::split<_<0, 3>, _<3, 3>>(opCode);

        d = Rdm;
        n = Rn;
        m = Rdm;
        setFlags = !cpu.isInItBlock();
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, Rd, Rn] = utils::split<_<0, 4>, _<8, 4>, _<16, 4>>(opCode);

        d = Rd;
        n = Rn;
        m = Rm;
        setFlags = false;

        UNPREDICTABLE_IF(d >= 13 || n >= 13 || m >= 13);
    }

    const auto result = cpu.R(n) * cpu.R(m);
    cpu.setR(d, result);

    if (setFlags) {
        APSR.N = utils::isNegative(result);
        APSR.Z = result == 0;
    }
}

template <bool substract>
void cmdMlaMls(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto [Rm, Rd, Ra, Rn] = utils::split<_<0, 4>, _<8, 4>, _<12, 4>, _<16, 4>>(opCode);

    uint32_t result;
    if constexpr (substract) {
        UNPREDICTABLE_IF(Rd >= 13 || Rn >= 13 || Rm >= 13 || Ra >= 13);
        result = cpu.R(Ra) - cpu.R(Rn) * cpu.R(Rm);
    }
    else {
        UNPREDICTABLE_IF(Rd >= 13 || Rn >= 13 || Rm >= 13 || Ra == 13);
        result = cpu.R(Rn) * cpu.R(Rm) + cpu.R(Ra);
    }

    cpu.setR(Rd, result);
}

template <bool isSigned>
void cmdMulLong(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto [Rm, RdHi, RdLo, Rn] = utils::split<_<0, 4>, _<8, 4>, _<12, 4>, _<16, 4>>(opCode);
    UNPREDICTABLE_IF(RdLo >= 13 || RdHi >= 13 || Rn >= 13 || Rm >= 13);
    UNPREDICTABLE_IF(RdHi == RdLo);

    using Type = std::conditional_t<isSigned, int64_t, uint64_t>;

    const uint64_t result = static_cast<uint64_t>(static_cast<Type>(cpu.R(Rn)) * static_cast<Type>(cpu.R(Rm)));
    cpu.setR(RdHi, utils::getPart<32, 32, uint32_t>(result));
    cpu.setR(RdLo, utils::getPart<0, 32, uint32_t>(result));
}

template <bool isSigned>
void cmdMulAccumulateLong(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto [Rm, RdHi, RdLo, Rn] = utils::split<_<0, 4>, _<8, 4>, _<12, 4>, _<16, 4>>(opCode);
    UNPREDICTABLE_IF(RdLo >= 13 || RdHi >= 13 || Rn >= 13 || Rm >= 13);
    UNPREDICTABLE_IF(RdHi == RdLo);

    using Type = std::conditional_t<isSigned, int64_t, uint64_t>;

    const auto acc = static_cast<Type>(utils::combine<uint64_t>(_<0, 32, uint32_t>{cpu.R(RdLo)}, _<32, 32, uint32_t>{cpu.R(RdHi)}));
    const uint64_t result = static_cast<uint64_t>(static_cast<Type>(cpu.R(Rn)) * static_cast<Type>(cpu.R(Rm)) + acc);
    cpu.setR(RdHi, utils::getPart<32, 32, uint32_t>(result));
    cpu.setR(RdLo, utils::getPart<0, 32, uint32_t>(result));
}

template <bool isSigned>
void cmdDiv(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto [Rm, Rd, Rn] = utils::split<_<0, 4>, _<8, 4>, _<16, 4>>(opCode);
    UNPREDICTABLE_IF(Rd >= 13 || Rn >= 13 || Rm >= 13);

    uint32_t result;

    using Type = std::conditional_t<isSigned, int32_t, uint32_t>;

    if (const auto m = cpu.R(Rm); m != Type{0}) {
        result = cpu.R(Rn) / m;
    }
    else {
        if (cpu.systemRegisters().CCR().DIV_0_TRP) {
            cpu.exceptionTaken(ExceptionType::UsageFault);
            return;
        }
        else {
            result = Type{0};
        }
    }
    cpu.setR(Rd, static_cast<uint32_t>(result));
}

template <Encoding encoding, Bitwise bitwise, typename T>
void cmdBitwiseRegister(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    CHECK_CONDITION;

    auto& APSR = cpu.registers().APSR();

    uint8_t d, n, setFlags;
    uint32_t shifted;
    bool carry;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rdn, Rm] = utils::split<_<0, 3>, _<3, 3>>(opCode);

        d = Rdn;
        n = Rdn;
        setFlags = !cpu.isInItBlock();

        shifted = cpu.R(Rm);
        carry = APSR.C;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, type, imm2, Rd, imm3, Rn, S] =
            utils::split<_<0, 4>, _<4, 2>, _<6, 2>, _<8, 4>, _<12, 3>, _<16, 4>, _<20, 1>>(opCode);

        const auto [shiftType, shiftN] = utils::decodeImmediateShift(type, utils::combine<uint8_t>(_<0, 2>{imm2}, _<2, 3>{imm3}));

        if constexpr (is_in<bitwise, Bitwise::AND, Bitwise::EOR>) {
            UNPREDICTABLE_IF(Rd == 13 || (Rd == 15 && S == 0) || Rn >= 13 || Rm >= 13);
        }
        else if constexpr (is_in<bitwise, Bitwise::ORR>) {
            UNPREDICTABLE_IF(Rd >= 13 || Rn == 13 || Rm >= 13);
        }
        else if constexpr (is_in<bitwise, Bitwise::BIC>) {
            UNPREDICTABLE_IF(Rd >= 13 || Rn >= 13 || Rm >= 13);
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

template <Bitwise bitwise>
void cmdBitwiseImmediate(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto [imm8, Rd, imm3, Rn, S, i] = utils::split<_<0, 8>, _<8, 4>, _<12, 3>, _<16, 4>, _<20, 1>, _<26, 1>>(opCode);
    if constexpr (is_in<bitwise, Bitwise::AND, Bitwise::EOR>) {
        UNPREDICTABLE_IF(Rd == 13 || (Rd == 15 && S == 0) || Rn >= 13);
    }
    else if constexpr (bitwise == Bitwise::ORR) {
        UNPREDICTABLE_IF(Rd >= 13 || Rn == 13);
    }
    else if constexpr (bitwise == Bitwise::BIC) {
        UNPREDICTABLE_IF(Rd >= 13 || Rn >= 13);
    }

    auto& APSR = cpu.registers().APSR();

    const auto [imm32, carry] =
        utils::thumbExpandImmediateWithCarry(utils::combine<uint16_t>(_<0, 8>{imm8}, _<8, 3>{imm3}, _<11, 1>{i}), APSR.C);

    uint32_t result;
    if constexpr (is_in<bitwise, Bitwise::AND>) {
        result = cpu.R(Rn) & imm32;
    }
    else if constexpr (is_in<bitwise, Bitwise::EOR>) {
        result = cpu.R(Rn) ^ imm32;
    }
    else if constexpr (is_in<bitwise, Bitwise::ORR>) {
        result = cpu.R(Rn) | imm32;
    }
    else if constexpr (is_in<bitwise, Bitwise::BIC>) {
        result = cpu.R(Rn) & ~imm32;
    }
    cpu.setR(Rd, result);

    if (S) {
        APSR.N = utils::isNegative(result);
        APSR.Z = result == 0;
        APSR.C = carry;
    }
}

template <Encoding encoding, typename T>
void cmdMvnRegister(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    CHECK_CONDITION;

    auto& APSR = cpu.registers().APSR();

    uint8_t d, setFlags;
    uint32_t shifted;
    bool carry;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rd, Rm] = utils::split<_<0, 3>, _<3, 3>>(opCode);

        d = Rd;
        setFlags = !cpu.isInItBlock();

        shifted = cpu.R(Rm);
        carry = APSR.C;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, type, imm2, Rd, imm3, S] = utils::split<_<0, 4>, _<4, 2>, _<6, 2>, _<8, 4>, _<12, 3>, _<20, 1>>(opCode);

        UNPREDICTABLE_IF(Rd >= 13 || Rm >= 13);

        const auto [shiftType, shiftN] = utils::decodeImmediateShift(type, utils::combine<uint8_t>(_<0, 2>{imm2}, _<2, 3>{imm3}));

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

inline void cmdMvnImmediate(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto [imm8, Rd, imm3, S, i] = utils::split<_<0, 8>, _<8, 4>, _<12, 3>, _<20, 1>, _<26, 1>>(opCode);
    UNPREDICTABLE_IF(Rd >= 13);

    auto& APSR = cpu.registers().APSR();

    const auto [imm32, carry] =
        utils::thumbExpandImmediateWithCarry(utils::combine<uint16_t>(_<0, 8>{imm8}, _<8, 3>{imm3}, _<11, 1>{i}), APSR.C);

    const auto result = ~imm32;
    cpu.setR(Rd, result);

    if (S) {
        APSR.N = utils::isNegative(result);
        APSR.Z = result == 0u;
        APSR.C = carry;
    }
}

template <Encoding encoding, typename Type, bool isSignExtended, typename T>
void cmdExtend(T opCode, Cpu& cpu)
{
    static_assert(std::is_same_v<Type, uint8_t> || std::is_same_v<Type, uint16_t>);
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    CHECK_CONDITION;

    uint8_t d;
    uint32_t rotated;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rd, Rm] = utils::split<_<0, 3>, _<3, 3>>(opCode);

        d = Rd;
        rotated = cpu.R(Rm);
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, rotate, Rd] = utils::split<_<0, 4>, _<4, 2>, _<8, 4>>(opCode);

        UNPREDICTABLE_IF(Rd >= 13 || Rm >= 13);

        d = Rd;
        const auto rotation = static_cast<uint8_t>(rotate << 3u);
        rotated = utils::ror(cpu.R(Rm), rotation);
    }

    uint32_t result;
    if constexpr (isSignExtended) {
        result = utils::signExtend<sizeof(Type) * 8>(rotated);
    }
    else {
        result = static_cast<uint32_t>(rotated);
    }

    cpu.setR(d, result);
}

template <Encoding encoding, typename Type, bool isSignExtended = false, typename T>
void cmdReverseBytes(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    CHECK_CONDITION;

    uint8_t d, m;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rd, Rm] = utils::split<_<0, 3>, _<3, 3>>(opCode);

        d = Rd;
        m = Rm;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, Rd, Rm_] = utils::split<_<0, 4>, _<8, 4>, _<16, 4>>(opCode);

        UNPREDICTABLE_IF(Rm != Rm_);

        d = Rd;
        m = Rm;

        UNPREDICTABLE_IF(d >= 13 || m >= 13);
    }

    uint32_t result;
    if constexpr (std::is_same_v<Type, uint32_t>) {
        result = utils::reverseEndianness(cpu.R(m));
    }
    else {
        if constexpr (isSignExtended) {
            const auto [lo, hi] = utils::split<_<0, 8>, _<8, 8>>(cpu.R(m));

            result = utils::combine<uint32_t>(_<0, 8>{hi}, _<8, 24, uint32_t>{utils::signExtend<8>(lo)});
        }
        else {
            const auto [lo, hi] = utils::split<_<0, 16, uint16_t>, _<16, 16, uint16_t>>(cpu.R(m));

            result = utils::combine<uint32_t>(_<0, 16, uint16_t>{utils::reverseEndianness(lo)},
                                              _<16, 16, uint16_t>{utils::reverseEndianness(hi)});
        }
    }

    cpu.setR(d, result);
}

inline void cmdReverseBits(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto [Rm, Rd, Rm_] = utils::split<_<0, 4>, _<8, 4>, _<16, 4>>(opCode);
    UNPREDICTABLE_IF(Rm != Rm_);
    UNPREDICTABLE_IF(Rd >= 13 || Rm >= 13);

    const auto result = utils::reverseBits(cpu.R(Rm));
    cpu.setR(Rd, result);
}

inline void cmdClz(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;
    const auto [Rm, Rd, Rm_] = utils::split<_<0, 4>, _<8, 4>, _<16, 4>>(opCode);
    UNPREDICTABLE_IF(Rm != Rm_);
    UNPREDICTABLE_IF(Rd >= 13 || Rm >= 13);

    const auto result = utils::countLeadingZeros(cpu.R(Rm));
    cpu.setR(Rd, result);
}

template <Encoding encoding, typename T>
void cmdMovImmediate(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2, Encoding::T3>);

    CHECK_CONDITION;

    auto& APSR = cpu.registers().APSR();

    uint8_t d, setFlags;
    uint32_t imm32;
    bool carry;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [imm8, Rd] = utils::split<_<0, 8, uint32_t>, _<8, 3>>(opCode);

        d = Rd;
        setFlags = !cpu.isInItBlock();
        imm32 = imm8;
        carry = APSR.C;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [imm8, Rd, imm3, S, i] = utils::split<_<0, 8>, _<8, 4>, _<12, 3>, _<20, 1>, _<26, 1>>(opCode);
        UNPREDICTABLE_IF(Rd >= 13);

        d = Rd;
        setFlags = S;
        std::tie(imm32, carry) =
            utils::thumbExpandImmediateWithCarry(utils::combine<uint16_t>(_<0, 8>{imm8}, _<8, 3>{imm3}, _<11, 1>{i}), APSR.C);
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>) {
        const auto [imm8, Rd, imm3, imm4, i] = utils::split<_<0, 8>, _<8, 4>, _<12, 3>, _<16, 4>, _<26, 1>>(opCode);

        d = Rd;
        setFlags = false;
        imm32 = utils::combine<uint32_t>(_<0, 8>{imm8}, _<8, 3>{imm3}, _<11, 1>{i}, _<12, 4>{imm4});
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

    CHECK_CONDITION;

    auto& APSR = cpu.registers().APSR();

    uint8_t d, m, setFlags;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rd, Rm, D] = utils::split<_<0, 3>, _<3, 4>, _<7, 1>>(opCode);

        d = utils::combine<uint8_t>(_<0, 3>{Rd}, _<3, 1>{D});
        UNPREDICTABLE_IF(d == 15 && cpu.isInItBlock() && !cpu.isLastInItBlock());

        m = Rm;
        setFlags = false;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint16_t, T>) {
        const auto [Rd, Rm] = utils::split<_<0, 3>, _<3, 3>>(opCode);

        UNPREDICTABLE_IF(cpu.isInItBlock());

        d = Rd;
        m = Rm;
        setFlags = true;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>) {
        const auto [Rm, Rd, S] = utils::split<_<0, 4>, _<8, 4>, _<20, 1>>(opCode);

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

inline void cmdMovt(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto [imm8, Rd, imm3, imm4, i] = utils::split<_<0, 8>, _<8, 4>, _<12, 3>, _<16, 4>, _<26, 1>>(opCode);
    UNPREDICTABLE_IF(Rd >= 13);

    const auto imm16 = utils::combine<uint16_t>(_<0, 8>{imm8}, _<8, 3>{imm3}, _<11, 1>{i}, _<12, 4>{imm4});

    const auto low = utils::getPart<0, 16, uint16_t>(cpu.R(Rd));
    const auto result = utils::combine<uint32_t>(_<0, 16, uint16_t>{low}, _<16, 16, uint16_t>{imm16});

    cpu.setR(Rd, result);
}

template <Encoding encoding, bool isNegative, typename T>
void cmdCmpImmediate(T opCode, Cpu& cpu)
{
    static_assert(encoding == Encoding::T1 || (encoding == Encoding::T2 && !isNegative));

    CHECK_CONDITION;

    auto& APSR = cpu.registers().APSR();

    uint8_t n;
    uint32_t imm32;
    if constexpr (!isNegative && is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [imm8, Rn] = utils::split<_<0, 8, uint32_t>, _<8, 3>>(opCode);

        n = Rn;
        imm32 = imm8;
    }
    if constexpr ((!isNegative && is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) ||
                  (isNegative && is_valid_opcode_encoding<Encoding::T1, encoding, uint32_t, T>)) {
        const auto [imm8, imm3, Rn, i] = utils::split<_<0, 8>, _<12, 3>, _<16, 4>, _<26, 1>>(opCode);
        UNPREDICTABLE_IF(Rn == 15);

        n = Rn;
        imm32 = utils::thumbExpandImmediateWithCarry(utils::combine<uint16_t>(_<0, 8>{imm8}, _<8, 3>{imm3}, _<11, 1>{i}), APSR.C).first;
    }

    if constexpr (!isNegative) {
        imm32 = ~imm32;
    }

    const auto [result, carry, overflow] = utils::addWithCarry(cpu.R(n), imm32, !isNegative);

    APSR.N = utils::isNegative(result);
    APSR.Z = result == 0;
    APSR.C = carry;
    APSR.V = overflow;
}

template <Encoding encoding, bool isNegative, typename T>
void cmdCmpRegister(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2> || (!isNegative && encoding == Encoding::T3));

    CHECK_CONDITION;

    auto& APSR = cpu.registers().APSR();

    uint8_t n;
    uint32_t shifted;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rn, Rm] = utils::split<_<0, 3>, _<3, 3>>(opCode);

        n = Rn;
        shifted = cpu.R(Rm);
    }
    else if constexpr (!isNegative && is_valid_opcode_encoding<Encoding::T2, encoding, uint16_t, T>) {
        const auto [Rn, Rm, N] = utils::split<_<0, 3>, _<3, 4>, _<7, 1>>(opCode);

        n = utils::combine<uint8_t>(_<0, 3>{Rn}, _<3, 1>{N});
        UNPREDICTABLE_IF(n < 8 && Rm < 8);
        UNPREDICTABLE_IF(n == 15 || Rm == 15);

        shifted = cpu.R(Rm);
    }
    else if constexpr ((!isNegative && is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>) ||
                       (isNegative && is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>)) {
        const auto [Rm, type, imm2, imm3, Rn] = utils::split<_<0, 4>, _<4, 2>, _<6, 2>, _<12, 3>, _<16, 4>>(opCode);

        const auto [shiftType, shiftN] = utils::decodeImmediateShift(type, utils::combine<uint8_t>(_<0, 2>{imm2}, _<2, 3>{imm3}));

        n = Rn;
        UNPREDICTABLE_IF(n == 15 || Rm >= 13);

        shifted = utils::shift(cpu.R(Rm), shiftType, shiftN, APSR.C);
    }

    if constexpr (!isNegative) {
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

    CHECK_CONDITION;

    auto& APSR = cpu.registers().APSR();

    uint8_t n;
    uint32_t shifted;
    bool carry;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rn, Rm] = utils::split<_<0, 3>, _<3, 3>>(opCode);

        n = Rn;
        shifted = cpu.R(Rm);
        carry = APSR.C;
    }
    if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, type, imm2, imm3, Rn] = utils::split<_<0, 4>, _<4, 2>, _<6, 2>, _<12, 3>, _<16, 4>>(opCode);
        UNPREDICTABLE_IF(Rn >= 13 || Rm >= 13);

        const auto [shiftType, shiftN] = utils::decodeImmediateShift(type, utils::combine<uint8_t>(_<0, 2>{imm2}, _<2, 3>{imm3}));

        n = Rn;
        std::tie(shifted, carry) = utils::shiftWithCarry(cpu.R(Rm), shiftType, shiftN, APSR.C);
    }

    const auto result = cpu.R(n) & shifted;

    APSR.N = utils::isNegative(result);
    APSR.Z = result == 0;
    APSR.C = carry;
}

inline void cmdTstImmediate(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto [imm8, imm3, Rn, i] = utils::split<_<0, 8>, _<12, 3>, _<16, 4>, _<26, 1>>(opCode);
    UNPREDICTABLE_IF(Rn >= 13);

    auto& APSR = cpu.registers().APSR();

    const auto [imm32, carry] =
        utils::thumbExpandImmediateWithCarry(utils::combine<uint16_t>(_<0, 8>{imm8}, _<8, 3>{imm3}, _<11, 1>{i}), APSR.C);

    const auto result = cpu.R(Rn) & imm32;

    APSR.N = utils::isNegative(result);
    APSR.Z = result == 0u;
    APSR.C = carry;
}

inline void cmdBfi(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto [msb, imm2, Rd, imm3, Rn] = utils::split<_<0, 5>, _<6, 2>, _<8, 4>, _<12, 3>, _<16, 4>>(opCode);
    UNPREDICTABLE_IF(Rd >= 13 || Rn == 13);

    const auto lsb = utils::combine<uint8_t>(_<0, 2>{imm2}, _<2, 3>{imm3});
    UNPREDICTABLE_IF(lsb < msb);

    const auto lowBits = utils::getPart<uint32_t>(cpu.R(Rn), 0u, static_cast<uint8_t>(static_cast<uint8_t>(msb - lsb) + 1u));

    const auto result = utils::clearBitField(cpu.R(Rd), lsb, msb) | (lowBits << lsb);
    cpu.setR(Rd, result);
}

inline void cmdBfc(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto [msb, imm2, Rd, imm3] = utils::split<_<0, 5>, _<6, 2>, _<8, 4>, _<12, 3>>(opCode);
    UNPREDICTABLE_IF(Rd >= 13);

    const auto lsb = utils::combine<uint8_t>(_<0, 2>{imm2}, _<2, 3>{imm3});
    UNPREDICTABLE_IF(lsb < msb);

    const auto result = utils::clearBitField(cpu.R(Rd), lsb, msb);
    cpu.setR(Rd, result);
}

template <bool isSigned>
void cmdBfx(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto [widthm1, imm2, Rd, imm3, Rn] = utils::split<_<0, 5>, _<6, 2>, _<8, 4>, _<12, 3>, _<16, 4>>(opCode);
    UNPREDICTABLE_IF(Rd >= 13 || Rn >= 13);

    const auto lsb = utils::combine<uint8_t>(_<0, 2>{imm2}, _<2, 3>{imm3});
    const auto msb = static_cast<uint8_t>(lsb + widthm1);
    UNPREDICTABLE_IF(msb > 31u);

    uint32_t result;
    if constexpr (isSigned) {
        const auto low = utils::getPart<uint32_t>(cpu.R(Rn), lsb, static_cast<uint8_t>(widthm1 + 1u));
        result = utils::signExtend(low, static_cast<uint8_t>(widthm1 + 1u));
    }
    else {
        result = utils::getPart<uint32_t>(cpu.R(Rn), lsb, static_cast<uint8_t>(widthm1 + 1u));
    }
    cpu.setR(Rd, result);
}

template <bool isSigned>
void cmdSat(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto [satImm, imm2, Rd, imm3, Rn, sh] = utils::split<_<0, 5>, _<6, 2>, _<8, 4>, _<12, 3>, _<16, 4>, _<21, 1>>(opCode);
    UNPREDICTABLE_IF(Rd >= 13 || Rn >= 13);

    const auto [shiftType, shiftN] =
        utils::decodeImmediateShift(static_cast<uint8_t>(sh << 2u), utils::combine<uint8_t>(_<0, 2>{imm2}, _<2, 3>{imm3}));

    auto& APSR = cpu.registers().APSR();

    const auto operand = static_cast<int32_t>(utils::shift(cpu.R(Rn), shiftType, shiftN, APSR.C));

    int32_t result;
    bool Q;
    if constexpr (isSigned) {
        std::tie(result, Q) = utils::signedSaturateQ(operand, static_cast<uint8_t>(satImm + 1u));
    }
    else {
        std::tie(result, Q) = utils::unsignedSaturateQ(operand, satImm);
    }
    APSR.Q |= Q;
}

template <bool withLink>
void cmdBranchAndExecuteRegister(uint16_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto m = utils::getPart<3, 4>(opCode);
    if constexpr (withLink) {
        UNPREDICTABLE_IF(m == 15);
    }
    UNPREDICTABLE_IF(cpu.isInItBlock() && !cpu.isLastInItBlock());

    if constexpr (withLink) {
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

    CHECK_CONDITION;

    uint32_t imm32;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [imm8, cond] = utils::split<_<0, 8, uint32_t>, _<8, 4>>(opCode);

        imm32 = utils::signExtend<9>(imm8 << 1u);
        UNPREDICTABLE_IF(cpu.isInItBlock());
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint16_t, T>) {
        const auto imm11 = utils::getPart<0, 11, uint32_t>(opCode);

        imm32 = utils::signExtend<12>(imm11 << 1u);
        UNPREDICTABLE_IF(cpu.isInItBlock() && !cpu.isLastInItBlock());
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>) {
        const auto [imm11, J2, J1, imm6, cond, S] =
            utils::split<_<0, 11, uint16_t>, _<11, 1>, _<13, 1>, _<16, 6>, _<22, 4>, _<26, 1>>(opCode);

        imm32 =
            utils::signExtend<21>(utils::combine<T>(_<1, 11, uint16_t>{imm11}, _<12, 6>{imm6}, _<18, 1>{J1}, _<19, 1>{J2}, _<20, 1>{S}));

        UNPREDICTABLE_IF(cpu.isInItBlock());
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T4, encoding, uint32_t, T>) {
        const auto [imm11, J2, J1, imm10, S] = utils::split<_<0, 11, uint16_t>, _<11, 1>, _<13, 1>, _<16, 10, uint16_t>, _<26, 1>>(opCode);

        const auto I1 = static_cast<uint8_t>(~(J1 ^ S));
        const auto I2 = static_cast<uint8_t>(~(J2 ^ S));

        imm32 = utils::signExtend<25>(
            utils::combine<uint32_t>(_<1, 11, uint16_t>{imm11}, _<12, 10, uint16_t>{imm10}, _<22, 1>{I2}, _<23, 1>{I1}, _<24, 1>{S}));

        UNPREDICTABLE_IF(cpu.isInItBlock() && !cpu.isLastInItBlock());
    }

    cpu.branchWritePC(cpu.currentInstructionAddress() + 4u + imm32);
}

inline void cmdBranchWithLinkImmediate(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto [imm11, J2, J1, imm10, S] = utils::split<_<0, 11, uint16_t>, _<11, 1>, _<13, 1>, _<16, 10, uint16_t>, _<26, 1>>(opCode);
    UNPREDICTABLE_IF(cpu.isInItBlock() && !cpu.isLastInItBlock());

    const auto I1 = static_cast<uint8_t>(~(J1 ^ S));
    const auto I2 = static_cast<uint8_t>(~(J2 ^ S));

    const auto imm32 = utils::signExtend<25>(
        utils::combine<uint32_t>(_<1, 11, uint16_t>{imm11}, _<12, 10, uint16_t>{imm10}, _<22, 1>{I2}, _<23, 1>{I1}, _<24, 1>{S}));

    const auto nextInstructionAddress = cpu.currentInstructionAddress() + 4u;
    cpu.registers().LR() = nextInstructionAddress | utils::ONES<1u, uint32_t>;
    cpu.branchWritePC(nextInstructionAddress + imm32);
}

inline void cmdCompareAndBranchOnZero(uint16_t opCode, Cpu& cpu)
{
    const auto [Rn, imm5, i, op] = utils::split<_<0, 3>, _<3, 5>, _<9, 1>, _<11, 1>>(opCode);

    const auto n = Rn;
    const auto imm32 = utils::combine<uint32_t>(_<1, 5>{imm5}, _<6, 1>{i});
    const auto nonZero = op;

    UNPREDICTABLE_IF(cpu.isInItBlock());

    if (nonZero != (cpu.R(n) == 0u)) {
        const auto PC = cpu.currentInstructionAddress() + 4u;
        cpu.branchWritePC(PC + imm32);
    }
}

inline void cmdIfThen(uint16_t opCode, Cpu& cpu)
{
    const auto [mask, firstCond] = utils::split<_<0, 4>, _<4, 4>>(opCode);

    UNPREDICTABLE_IF(firstCond == 0b1111u || (firstCond == 0b111u && utils::bitCount(mask) != 1u));
    UNPREDICTABLE_IF(cpu.isInItBlock());

    const auto ITSTATE = utils::combine<uint8_t>(_<0, 4>{mask}, _<4, 4>{firstCond});
    cpu.registers().setITSTATE(ITSTATE);
}

inline void cmdMsr(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto [SYSm, mask, Rn] = utils::split<_<0, 8>, _<10, 2>, _<16, 4>>(opCode);
    UNPREDICTABLE_IF(mask == 0 || (mask != 0b10u && !(SYSm >= 0 && SYSm <= 3)));
    UNPREDICTABLE_IF(Rn >= 13 || (SYSm > 3 && SYSm < 5) || (SYSm > 9 && SYSm < 16) || SYSm > 20);

    switch (utils::getPart<3, 5>(SYSm)) {
        case 0b0000u:
            if (utils::isBitClear<2>(SYSm)) {
                UNPREDICTABLE_IF(utils::isBitSet<0>(mask));
                if (utils::isBitSet<1>(mask)) {
                    auto& APSR = cpu.registers().APSR();
                    APSR.registerData = utils::copyPartInto<_<27, 5>, _<27, 5>>(cpu.R(Rn), APSR.registerData);
                }
            }
            break;
        case 0b00001u:
            if (cpu.isInPrivilegedMode()) {
                switch (utils::getPart<0, 3>(SYSm)) {
                    case 0b000u:
                        cpu.registers().SP_main() = cpu.R(Rn);
                        break;
                    case 0b001u:
                        cpu.registers().SP_process() = cpu.R(Rn);
                        break;
                    default:
                        break;
                }
            }
            break;
        case 0b00010u:
            switch (utils::getPart<0, 3>(SYSm)) {
                case 0b000u:
                    if (cpu.isInPrivilegedMode()) {
                        cpu.registers().PRIMASK().registerData = cpu.R(Rn);
                    }
                    break;
                case 0b001u:
                    if (cpu.isInPrivilegedMode()) {
                        cpu.registers().BASEPRI().registerData = cpu.R(Rn);
                    }
                    break;
                case 0b010u:
                    if (cpu.isInPrivilegedMode()) {
                        auto& BASEPRI = cpu.registers().BASEPRI();
                        const auto value = utils::getPart<0, 8>(cpu.R(Rn));

                        if (cpu.isInPrivilegedMode() && value != 0u && (value < BASEPRI.level || BASEPRI.level == 0)) {
                            BASEPRI.level = value;
                        }
                    }
                    break;
                case 0b011u:
                    if (cpu.isInPrivilegedMode() && cpu.executionPriority() > -1) {
                        cpu.registers().FAULTMASK().registerData = cpu.R(Rn);
                    }
                    break;
                case 0b100u:
                    if (cpu.isInPrivilegedMode()) {
                        auto& CONTROL = cpu.registers().CONTROL();
                        const auto value = cpu.R(Rn);
                        CONTROL.nPRIV = utils::isBitSet<0>(value);
                        if (cpu.currentMode() == ExecutionMode::Thread) {
                            cpu.registers().CONTROL().SPSEL = utils::isBitSet<1>(value);
                        }
                    }
                    break;
                default:
                    break;
            }
    }
}

inline void cmdMrs(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto [SYSm, Rd] = utils::split<_<0, 8>, _<8, 4>>(opCode);
    UNPREDICTABLE_IF(Rd >= 13 || (SYSm > 3 && SYSm < 5) || (SYSm > 9 && SYSm < 16) || SYSm > 20);

    uint32_t result{};
    switch (utils::getPart<3, 5>(SYSm)) {
        case 0b00000u:
            if (utils::isBitSet<0>(SYSm)) {
                result = cpu.registers().IPSR().registerData;
            }
            if (utils::isBitClear<2>(SYSm)) {
                result = cpu.registers().APSR().registerData;
            }
            break;
        case 0b00001u:
            if (cpu.isInPrivilegedMode()) {
                switch (utils::getPart<0, 3>(SYSm)) {
                    case 0b000u:
                        result = cpu.registers().SP_main();
                        break;
                    case 0b001u:
                        result = cpu.registers().SP_process();
                        break;
                    default:
                        break;
                }
            }
            break;
        case 0b00010u:
            switch (utils::getPart<0, 3>(SYSm)) {
                case 0b000u:
                    result = cpu.isInPrivilegedMode() ? cpu.registers().PRIMASK().registerData : 0u;
                    break;
                case 0b001u:
                case 0b010u:
                    result = cpu.isInPrivilegedMode() ? cpu.registers().BASEPRI().registerData : 0u;
                    break;
                case 0b011u:
                    result = cpu.isInPrivilegedMode() ? cpu.registers().FAULTMASK().registerData : 0u;
                    break;
                case 0b100u: {
                    auto& CONTROL = cpu.registers().CONTROL();
                    result = utils::combine<uint32_t>(_<0, 1>{CONTROL.nPRIV}, _<1, 1>{CONTROL.SPSEL});
                    break;
                }
                default:
                    break;
            }
    }

    cpu.setR(Rd, result);
}

template <Hint /*hint*/, typename T>
void cmdHint(T /*opCode*/, Cpu& cpu)
{
    CHECK_CONDITION;

    // TODO: implement hint's logic
}

template <Control control>
void cmdMiscControl(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto option = utils::getPart<0, 4>(opCode);
    if constexpr (control == Control::InstructionSynchronizationBarrier) {
        UNPREDICTABLE_IF(cpu.isInItBlock());
    }

    if constexpr (control == Control::ClearExclusive) {
        UNUSED(option);
        UNIMPLEMENTED;
    }
    else if constexpr (control == Control::DataSynchronizationBarrier) {
        cpu.dataSynchronizationBarrier(option);
    }
    else if constexpr (control == Control::DataMemoryBarrier) {
        cpu.dataMemoryBarrier(option);
    }
    else if constexpr (control == Control::InstructionSynchronizationBarrier) {
        cpu.instructionSynchronizationBarrier(option);
    }
}

template <Encoding encoding, typename T>
void cmdPermanentlyUndefined(T /*opCode*/, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    CHECK_CONDITION;

    // TODO: raise UNDEFINED
}

inline void cmdCallSupervisor(uint16_t /*opCode*/, Cpu& cpu)
{
    CHECK_CONDITION;

    // TODO: call supervisor
}

template <Encoding encoding, typename T>
void cmdAdr(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2, Encoding::T3>);

    CHECK_CONDITION;

    uint8_t d, add;
    uint32_t imm32;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [imm8, Rd] = utils::split<_<0, 8, uint32_t>, _<8, 3>>(opCode);

        d = Rd;
        imm32 = imm8;
        add = true;

        UNPREDICTABLE_IF(d >= 13);
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T> ||
                       is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>) {
        const auto [imm8, Rd, imm3, S, i] = utils::split<_<0, 8>, _<8, 4>, _<12, 3>, _<23, 1>, _<26, 1>>(opCode);

        d = Rd;
        imm32 = utils::combine<T>(_<0, 8>{imm8}, _<8, 3>{imm3}, _<11, 1>{i});
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
void cmdLoadLiteral(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    CHECK_CONDITION;

    uint8_t t, add;
    uint32_t imm32;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [imm8, Rt] = utils::split<_<0, 8, uint32_t>, _<8, 3>>(opCode);

        t = Rt;
        imm32 = static_cast<uint32_t>(imm8 << 2u);
        add = true;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [imm12, Rt, U] = utils::split<_<0, 12, uint32_t>, _<12, 4>, _<23, 1>>(opCode);

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
        UNPREDICTABLE_IF((utils::getPart<0, 2>(data)));
        cpu.loadWritePC(data);
    }
    else {
        cpu.setR(t, data);
    }
}

template <Encoding encoding, typename T>
inline void cmdPush(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2, Encoding::T3>);

    CHECK_CONDITION;

    uint16_t registers;
    // bool unalignedAllowed; // TODO: check unaligned write
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [registerList, M] = utils::split<_<0, 7>, _<8, 1>>(opCode);

        registers = utils::combine<uint16_t>(_<0, 8>{registerList}, _<14, 1>{M});
        // unalignedAllowed = false;
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [registerList, M] = utils::split<_<0, 13, uint16_t>, _<14, 1>>(opCode);

        registers = utils::combine<uint16_t>(_<0, 13, uint16_t>{registerList}, _<15, 1>{M});
        // unalignedAllowed = false;

        UNPREDICTABLE_IF((utils::bitCount(registers) < 2u));
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>) {
        const auto Rt = utils::getPart<12, 4>(opCode);

        UNPREDICTABLE_IF(Rt >= 13);

        registers = 0b1u << Rt;
        // unalignedAllowed = true;
    }

    uint32_t bottomAddress = cpu.registers().SP() - 4u * utils::bitCount(registers);

    uint32_t address = bottomAddress;
    for (uint8_t i = 0; i < 15u; registers = static_cast<uint16_t>(registers >> 1u), ++i) {
        if ((registers & 0b1u) == 0u) {
            continue;
        }

        cpu.memory().write(address, cpu.R(i));
        address += 4u;
    }

    cpu.registers().SP() = bottomAddress;
}

template <Encoding encoding, typename T>
inline void cmdPop(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2, Encoding::T3>);

    CHECK_CONDITION;

    uint16_t registers;
    // bool unalignedAccess;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [registerList, P] = utils::split<_<0, 8>, _<8, 1>>(opCode);

        registers = utils::combine<uint16_t>(_<0, 8>{registerList}, _<15, 1>{P});
        // unalignedAccess = false;

        UNPREDICTABLE_IF(utils::isBitSet<15>(registers) && cpu.isInItBlock() && !cpu.isLastInItBlock());
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [registerList, M, P] = utils::split<_<0, 13, uint16_t>, _<14, 1>, _<15, 1>>(opCode);

        registers = utils::combine<uint16_t>(_<0, 13, uint16_t>{registerList}, _<14, 1>{M}, _<15, 1>{P});
        // unalignedAccess = false;

        UNPREDICTABLE_IF(utils::bitCount(registers) < 2u || (P == 1u && M == 1u));
        UNPREDICTABLE_IF(utils::isBitSet<15>(registers) && cpu.isInItBlock() && !cpu.isLastInItBlock());
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>) {
        const auto Rt = utils::getPart<12, 4>(opCode);

        UNPREDICTABLE_IF(Rt >= 13 || (Rt == 15 && cpu.isInItBlock() && !cpu.isLastInItBlock()));

        registers = 0b1u << Rt;
        // unalignedAllowed = true;
    }

    uint32_t address = cpu.registers().SP();

    cpu.registers().SP() = cpu.registers().SP() + 4 * utils::bitCount(registers);

    for (uint8_t i = 0; i < 15u; registers = static_cast<uint16_t>(registers >> 1u), ++i) {
        if ((registers & 0b1u) == 0u) {
            continue;
        }

        cpu.setR(i, cpu.memory().read<uint32_t>(address));
        address += 4u;
    }

    if (registers & 0b1u) {
        cpu.loadWritePC(cpu.memory().read<uint32_t>(address));
    }
}

template <Encoding encoding, typename T>
void cmdStoreMultiple(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    CHECK_CONDITION;

    uint8_t n, writeBack;
    uint16_t registers;
    uint32_t registerCount;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [registerList, Rn] = utils::split<_<0, 8>, _<8, 3>>(opCode);

        n = Rn;
        registers = static_cast<uint16_t>(registerList);
        writeBack = true;

        registerCount = utils::bitCount(registers);

        UNPREDICTABLE_IF(registerCount < 1);
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [registerList, M, Rn, W] = utils::split<_<0, 13, uint16_t>, _<14, 1>, _<16, 4>, _<21, 1>>(opCode);

        n = Rn;
        registers = utils::combine<uint16_t>(_<0, 13, uint16_t>{registerList}, _<14, 1>{M});
        writeBack = W;

        registerCount = utils::bitCount(registers);

        UNPREDICTABLE_IF(writeBack && utils::isBitSet(registers, n));
    }

    auto address = cpu.R(n);

    auto lowestSetBit = static_cast<uint8_t>(utils::lowestSetBit(registers));

    for (uint8_t i = 0; i < 15u; registers = static_cast<uint16_t>(registers >> 1u), ++i) {
        if ((registers & 0b1u) == 0u) {
            continue;
        }

        if constexpr (is_in<encoding, Encoding::T1>) {
            if (i == n && writeBack && i != lowestSetBit) {
                continue;
            }
        }

        cpu.mpu().alignedMemoryWrite(address, cpu.R(i));
        address += 4u;
    }

    if (writeBack) {
        const auto result = cpu.R(n) + 4 * registerCount;
        cpu.setR(n, result);
    }
}

template <Encoding encoding, typename T>
void cmdLoadMultiple(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    CHECK_CONDITION;

    uint8_t n, writeBack;
    uint16_t registers;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [registerList, Rn] = utils::split<_<0, 8>, _<8, 3>>(opCode);

        n = Rn;
        registers = static_cast<uint16_t>(registerList);
        writeBack = utils::isBitClear(registers, n);

        UNPREDICTABLE_IF(utils::bitCount(registers));
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [registerList, M, P, Rn, W] = utils::split<_<0, 13, uint16_t>, _<14, 1>, _<15, 1>, _<16, 4>, _<21, 1>>(opCode);

        n = Rn;
        registers = utils::combine<uint16_t>(_<0, 13, uint16_t>{registerList}, _<14, 1>{M}, _<15, 1>{P});
        writeBack = W;

        UNPREDICTABLE_IF(n == 15 || utils::bitCount(registers) < 2 || (P != 0u && M != 0u));
        UNPREDICTABLE_IF(utils::isBitSet<15>(registers) && cpu.isInItBlock() && !cpu.isLastInItBlock());
        UNPREDICTABLE_IF(writeBack && utils::isBitSet(registers, n));
    }

    auto address = cpu.R(n);

    for (uint8_t i = 0; i < 15u; registers = static_cast<uint16_t>(registers >> 1u), ++i) {
        if ((registers & 0b1u) == 0u) {
            continue;
        }

        cpu.setR(i, cpu.mpu().alignedMemoryRead<uint32_t>(address));
        address += 4u;
    }

    if (utils::isBitSet<15>(registers)) {
        cpu.loadWritePC(cpu.mpu().alignedMemoryRead<uint32_t>(address));
    }

    if (writeBack && utils::isBitClear(registers, n)) {
        const auto result = cpu.R(n) + 4 * utils::bitCount(registers);
        cpu.setR(n, result);
    }
}

template <Encoding encoding, typename Type, typename T>
void cmdStoreRegister(T opCode, Cpu& cpu)
{
    static_assert(std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> || std::is_same_v<T, uint32_t>);
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    CHECK_CONDITION;

    uint8_t t, n;
    uint32_t offset;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rt, Rn, Rm] = utils::split<_<0, 3>, _<3, 3>, _<6, 3>>(opCode);

        t = Rt;
        n = Rn;
        offset = cpu.R(Rm);
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, imm2, Rt, Rn] = utils::split<_<0, 4>, _<4, 2>, _<12, 4>, _<16, 4>>(opCode);

        UNPREDICTABLE_IF(Rm >= 13);

        t = Rt;
        n = Rn;
        offset = static_cast<uint32_t>(cpu.R(Rm) << imm2);

        if constexpr (std::is_same_v<Type, uint32_t>) {
            UNPREDICTABLE_IF(t == 15);
        }
        else {
            UNPREDICTABLE_IF(t >= 13);
        }
    }

    const auto address = cpu.R(n) + offset;
    if constexpr (std::is_same_v<Type, uint32_t>) {
        cpu.mpu().unalignedMemoryWrite(address, cpu.R(t));
    }
    else if constexpr (std::is_same_v<Type, uint16_t>) {
        cpu.mpu().unalignedMemoryWrite(address, utils::getPart<0, 16, uint16_t>(cpu.R(t)));
    }
    else if constexpr (std::is_same_v<Type, uint8_t>) {
        cpu.mpu().unalignedMemoryWrite(address, utils::getPart<0, 8, uint8_t>(cpu.R(t)));
    }
}

template <Encoding encoding, typename Type, bool isSignExtended = false, typename T>
void cmdLoadRegister(T opCode, Cpu& cpu)
{
    static_assert(std::is_same_v<Type, uint8_t> || std::is_same_v<Type, uint16_t> || std::is_same_v<Type, uint32_t>);
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2>);

    CHECK_CONDITION;

    uint8_t t, n;
    uint32_t offset;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rt, Rn, Rm] = utils::split<_<0, 3>, _<3, 3>, _<6, 3>>(opCode);

        t = Rt;
        n = Rn;
        offset = cpu.R(Rm);
    }
    else if constexpr (is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>) {
        const auto [Rm, imm2, Rt, Rn] = utils::split<_<0, 4>, _<4, 2>, _<12, 4>, _<16, 4>>(opCode);

        UNPREDICTABLE_IF(Rm >= 13);

        t = Rt;
        n = Rn;
        offset = static_cast<uint32_t>(cpu.R(Rm) << imm2);

        if constexpr (std::is_same_v<Type, uint32_t>) {
            UNPREDICTABLE_IF(t == 15 && cpu.isInItBlock() && !cpu.isLastInItBlock());
        }
        else {
            UNPREDICTABLE_IF(t == 15);
        }
    }

    const auto address = cpu.R(n) + offset;
    uint32_t data;
    if constexpr (std::is_same_v<Type, uint32_t>) {
        data = cpu.mpu().unalignedMemoryRead<uint32_t>(address);
    }
    else if constexpr (std::is_same_v<Type, uint16_t>) {
        data = static_cast<uint32_t>(cpu.mpu().unalignedMemoryRead<uint16_t>(address));

        if constexpr (isSignExtended) {
            data = utils::signExtend<16>(data);
        }
    }
    else if constexpr (std::is_same_v<Type, uint8_t>) {
        data = static_cast<uint32_t>(cpu.mpu().unalignedMemoryRead<uint8_t>(address));

        if constexpr (isSignExtended) {
            data = utils::signExtend<8>(data);
        }
    }

    if (t == 15) {
        UNPREDICTABLE_IF((utils::getPart<0, 2>(data)));
        cpu.loadWritePC(static_cast<uint32_t>(data));
    }
    else {
        cpu.setR(t, data);
    }
}

inline void cmdLoadRegisterUnprivileged(uint32_t opCode, Cpu& cpu)
{
    CHECK_CONDITION;

    const auto [imm8, Rt, Rn] = utils::split<_<0, 8>, _<12, 4>, _<16, 4>>(opCode);
    UNPREDICTABLE_IF(Rt >= 13);

    const auto imm32 = static_cast<uint32_t>(imm8);

    const auto address = cpu.R(Rn) + imm32;
    const auto data = cpu.mpu().unalignedMemoryRead<uint32_t>(address, AccessType::Unprivileged);

    cpu.setR(Rt, data);
}

template <Encoding encoding, typename Type, typename T>
void cmdStoreImmediate(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2, Encoding::T3> ||
                  (std::is_same_v<Type, uint32_t> && is_in<encoding, Encoding::T4>));

    CHECK_CONDITION;

    uint8_t t, n, index, add, writeBack;
    uint32_t imm32;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rt, Rn, imm5] = utils::split<_<0, 3>, _<3, 3>, _<6, 5, uint32_t>>(opCode);

        t = Rt;
        n = Rn;
        imm32 = static_cast<uint32_t>(opCode << utils::getAlignmentBitCount<Type>());
        index = true;
        add = true;
        writeBack = false;
    }
    else if constexpr (std::is_same_v<Type, uint32_t> && is_valid_opcode_encoding<Encoding::T2, encoding, uint16_t, T>) {
        const auto [imm8, Rt] = utils::split<_<0, 8, uint32_t>, _<8, 3>>(opCode);

        t = Rt;
        n = rg::RegisterType::SP;
        imm32 = static_cast<uint32_t>(imm8 << 2u);
        index = true;
        add = true;
        writeBack = false;
    }
    else if constexpr ((std::is_same_v<Type, uint32_t> && is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>) ||
                       (!std::is_same_v<Type, uint32_t> && is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>)) {
        const auto [imm12, Rt, Rn] = utils::split<_<0, 12, uint32_t>, _<12, 4>, _<16, 4>>(opCode);

        t = Rt;
        n = Rn;
        imm32 = imm12;
        index = true;
        add = true;
        writeBack = false;

        if constexpr (std::is_same_v<Type, uint32_t>) {
            UNPREDICTABLE_IF(t == 15);
        }
        else {
            UNPREDICTABLE_IF(t >= 13);
        }
    }
    else if constexpr ((std::is_same_v<Type, uint32_t> && is_valid_opcode_encoding<Encoding::T4, encoding, uint32_t, T>) ||
                       (!std::is_same_v<Type, uint32_t> && is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>)) {
        const auto [imm8, W, U, P, Rt, Rn] = utils::split<_<0, 8>, _<8, 1>, _<9, 1>, _<10, 1>, _<12, 4>, _<16, 4>>(opCode);

        t = Rt;
        n = Rn;
        imm32 = static_cast<uint32_t>(imm8);
        index = P;
        add = U;
        writeBack = W;

        UNPREDICTABLE_IF(writeBack && n == t);
        if constexpr (std::is_same_v<Type, uint32_t>) {
            UNPREDICTABLE_IF(t == 15);
        }
        else {
            UNPREDICTABLE_IF(t >= 13);
        }
    }

    const auto offsetAddress = add ? (cpu.R(n) + imm32) : (cpu.R(n) - imm32);
    const auto address = index ? offsetAddress : cpu.R(n);

    if constexpr (std::is_same_v<Type, uint32_t>) {
        cpu.mpu().unalignedMemoryWrite(address, cpu.R(t));
    }
    else if constexpr (std::is_same_v<Type, uint16_t>) {
        cpu.mpu().unalignedMemoryWrite(address, utils::getPart<0, 16, uint16_t>(cpu.R(t)));
    }
    else if constexpr (std::is_same_v<Type, uint8_t>) {
        cpu.mpu().unalignedMemoryWrite(address, utils::getPart<0, 8, uint8_t>(cpu.R(t)));
    }

    if (writeBack) {
        cpu.setR(n, offsetAddress);
    }
}

template <Encoding encoding, typename Type, typename T>
void cmdLoadImmediate(T opCode, Cpu& cpu)
{
    static_assert(is_in<encoding, Encoding::T1, Encoding::T2, Encoding::T3> ||
                  (std::is_same_v<Type, uint32_t> && is_in<encoding, Encoding::T4>));

    CHECK_CONDITION;

    uint8_t t, n, index, add, writeBack;
    uint32_t imm32;
    if constexpr (is_valid_opcode_encoding<Encoding::T1, encoding, uint16_t, T>) {
        const auto [Rt, Rn, imm5] = utils::split<_<0, 3>, _<3, 3>, _<6, 5, uint32_t>>(opCode);

        t = Rt;
        n = Rn;
        imm32 = static_cast<uint32_t>(opCode << utils::getAlignmentBitCount<Type>());
        index = true;
        add = true;
        writeBack = false;
    }
    else if constexpr (std::is_same_v<Type, uint32_t> && is_valid_opcode_encoding<Encoding::T2, encoding, uint16_t, T>) {
        const auto [imm8, Rt] = utils::split<_<0, 8, uint32_t>, _<8, 3>>(opCode);

        t = Rt;
        n = rg::RegisterType::SP;
        imm32 = static_cast<uint32_t>(imm8 << 2u);
        index = true;
        add = true;
        writeBack = false;
    }
    else if constexpr ((std::is_same_v<Type, uint32_t> && is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>) ||
                       (!std::is_same_v<Type, uint32_t> && is_valid_opcode_encoding<Encoding::T2, encoding, uint32_t, T>)) {
        const auto [imm12, Rt, Rn] = utils::split<_<0, 12, uint16_t>, _<12, 4>, _<16, 4>>(opCode);

        t = Rt;
        n = Rn;
        imm32 = static_cast<uint32_t>(imm12);
        index = true;
        add = true;
        writeBack = false;

        if constexpr (std::is_same_v<Type, uint32_t>) {
            UNPREDICTABLE_IF(t == 15 && cpu.isInItBlock() && !cpu.isLastInItBlock());
        }
        else {
            UNPREDICTABLE_IF(t == 13);
        }
    }
    else if constexpr ((std::is_same_v<Type, uint32_t> && is_valid_opcode_encoding<Encoding::T4, encoding, uint32_t, T>) ||
                       (!std::is_same_v<Type, uint32_t> && is_valid_opcode_encoding<Encoding::T3, encoding, uint32_t, T>)) {
        const auto [imm8, W, U, P, Rt, Rn] = utils::split<_<0, 8>, _<8, 1>, _<9, 1>, _<10, 1>, _<12, 4>, _<16, 4>>(opCode);

        t = Rt;
        n = Rn;
        imm32 = static_cast<uint32_t>(imm8);
        index = P;
        add = U;
        writeBack = W;

        UNPREDICTABLE_IF(writeBack && n == t);
        if constexpr (std::is_same_v<Type, uint32_t>) {
            UNPREDICTABLE_IF(t == 15 && cpu.isInItBlock() && !cpu.isLastInItBlock());
        }
        else if constexpr (std::is_same_v<Type, uint16_t>) {
            UNPREDICTABLE_IF((t == 13) || t == 15 && (W == 1));
        }
        else if constexpr (std::is_same_v<Type, uint8_t>) {
            UNPREDICTABLE_IF((t == 13) || t == 15 && (P == 0 || U == 1 || W == 1));
        }
    }

    const auto offsetAddress = add ? (cpu.R(n) + imm32) : (cpu.R(n) - imm32);
    const auto address = index ? offsetAddress : cpu.R(n);

    const auto data = cpu.mpu().unalignedMemoryRead<Type>(address);
    if (writeBack) {
        cpu.setR(n, offsetAddress);
    }

    if (t == 15) {
        UNPREDICTABLE_IF((utils::getPart<0, 2>(data)));
        cpu.loadWritePC(static_cast<uint32_t>(data));
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

    const auto [F, I, im] = utils::split<_<0, 1>, _<1, 1>, _<4, 1>>(opCode);

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
