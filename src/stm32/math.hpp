#pragma once

#include <cassert>
#include <cstdint>
#include <limits>
#include <tuple>
#include <utility>

namespace stm32::math
{
template <typename T, typename R = T>
constexpr R MAX_SHIFT = static_cast<R>(sizeof(T) * 8u - 1u);

template <typename T, typename R = T>
constexpr R LEFT_BIT = static_cast<R>(T(0b1u) << MAX_SHIFT<T>);
template <typename T, typename R = T>
constexpr R RIGHT_BIT = T(0b1u);
template <typename T, typename R = T>
constexpr R ALL_BITS = static_cast<R>(~T(0b0u));

template <typename T>
inline auto lsl_c(T x, int8_t shift) -> std::pair<T, bool>
{
    assert(shift > 0);
    const auto carry = x & LEFT_BIT<T>;
    return {x << shift, carry};
}

template <typename T>
inline auto lsl(T x, int8_t shift) -> T
{
    assert(shift >= 0);
    return x << shift;
}

template <typename T>
inline auto lsr_c(T x, int8_t shift) -> std::pair<T, bool>
{
    assert(shift > 0);
    const auto carry = x & RIGHT_BIT<T>;
    return {x >> shift, carry};
}

template <typename T>
inline auto lsr(T x, int8_t shift) -> T
{
    assert(shift >= 0);
    return x >> shift;
}

template <typename T>
inline auto asr_c(T x, int8_t shift) -> std::pair<T, bool>
{
    assert(shift > 0);
    const auto carry = x & RIGHT_BIT<T>;
    if (x & LEFT_BIT<T>)
    {
        return {x >> shift | ~(ALL_BITS<T> >> shift), carry};
    }
    else
    {
        return {x >> shift, carry};
    }
}

template <typename T>
inline auto asr(T x, int8_t shift) -> T
{
    assert(shift >= 0);
    if (x & LEFT_BIT<T>)
    {
        return x >> shift | ~(ALL_BITS<T> >> shift);
    }
    else
    {
        return x >> shift;
    }
}

template <typename T>
inline auto ror_c(T x, int8_t shift) -> std::pair<T, bool>
{
    assert(shift != 0);
    shift &= MAX_SHIFT<T>;
    const auto result = (x >> shift) | (x << (-shift & MAX_SHIFT<T>));
    return {result, result & LEFT_BIT<T>};
}

template <typename T>
inline auto ror(T x, int8_t shift) -> T
{
    if (shift == 0)
    {
        return x;
    }

    shift &= MAX_SHIFT<T>;
    return (x >> shift) | (x << (-shift & MAX_SHIFT<T>));
}

template <typename T>
inline auto rrx_c(T x, bool carryIn) -> std::pair<T, bool>
{
    const auto carryOut = x & RIGHT_BIT<T>;
    return {(carryIn << MAX_SHIFT<T>) | (x >> 1u), carryOut};
}

template <typename T>
inline auto rrx(T x, bool carryIn) -> T
{
    return (carryIn << MAX_SHIFT<T>) | (x >> 1u);
}

enum class ShiftType
{
    LSL,
    LSR,
    ASR,
    RRX,
    ROR,
};

inline auto decodeImmediateShift(uint8_t bits, uint16_t immediate) -> std::pair<ShiftType, uint8_t>
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
            return {};
    }
}

template <typename T>
inline auto shiftCarry(T value, ShiftType shiftType, int8_t shift, bool carryIn) -> std::pair<T, bool>
{
    assert(shiftType != ShiftType::RRX || shift == 1u);

    if (shift == 0)
    {
        return {value, carryIn};
    }

    switch (shiftType)
    {
        case ShiftType::LSL:
            return lsl_c(value, shift);
        case ShiftType::LSR:
            return lsr_c(value, shift);
        case ShiftType::ASR:
            return asr_c(value, shift);
        case ShiftType::ROR:
            return ror_c(value, shift);
        case ShiftType::RRX:
            return rrx_c(value, carryIn);
    }
}

inline auto addWithCarry(uint32_t x, uint32_t y, bool carryIn) -> std::tuple<uint32_t, bool, bool>
{
    const auto sum = static_cast<uint64_t>(x) + static_cast<uint64_t>(y) + static_cast<uint64_t>(carryIn);

    const auto carryOut = sum >> (MAX_SHIFT<uint32_t, uint64_t> + 1u);

    const auto signSum = sum & LEFT_BIT<uint32_t, uint64_t>;
    const auto signX = x & LEFT_BIT<uint32_t>;
    const auto signY = y & LEFT_BIT<uint32_t>;
    const auto overflow = signX == signY && signX != signSum;

    return {static_cast<uint32_t>(sum), carryOut, overflow};
}

}  // namespace stm32::math
