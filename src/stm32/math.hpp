#pragma once

#include <cassert>
#include <cstdint>
#include <limits>
#include <tuple>
#include <utility>

#include "utils.hpp"

namespace stm32::math
{
template <typename T, typename R = T>
constexpr R MAX_SHIFT = static_cast<R>(sizeof(T) * 8u - 1u);

template <typename T, typename R = T>
constexpr R LEFT_BIT = static_cast<R>(T(0b1u) << MAX_SHIFT<T>);

template <uint8_t offset, typename T, typename R = T>
constexpr R BIT = static_cast<R>(T(0b1u) << offset);

template <typename T, typename R = T>
constexpr R RIGHT_BIT = T(0b1u);

template <typename T, typename R = T>
constexpr R ALL_BITS = static_cast<R>(~T(0b0u));

template <uint8_t N, typename T>
constexpr T ONES = (T(0b1u) << N) - T(0b1u);

template <uint8_t /* offset */, uint8_t /* bitCount */, typename T = uint8_t>
struct Part {
    T value;
};

namespace details
{
template <typename, typename>
struct Extractor;

template <typename V, uint8_t offset, uint8_t bitCount, typename T>
struct Extractor<V, Part<offset, bitCount, T>> {
    using Result = T;
    static constexpr auto create(const T& part) -> V { return (static_cast<V>(part) & ONES<bitCount, V>) << offset; }
    static constexpr auto extract(const V& value) -> T { return static_cast<T>(value >> offset) & ONES<bitCount, T>; }
};
}  // namespace details

template <typename V, typename... Parts>
auto split(const V& value) -> std::tuple<typename details::Extractor<V, Parts>::Result...>
{
    return std::tuple(details::Extractor<V, Parts>::extract(value)...);
}

template <typename V, typename... Parts>
auto combine(Parts... part) -> V
{
    return (details::Extractor<V, Parts>::create(part.value) | ...);
}

template <uint8_t Offset, uint8_t N, typename V, typename R = V>
constexpr auto getPart(const V& value) -> R
{
    return static_cast<V>(value >> Offset) & ONES<N, V>;
}

template <typename T>
constexpr auto isNegative(const T& value) -> bool
{
    return value & LEFT_BIT<T>;
}

template <typename T>
inline auto lslWithCarry(T x, int8_t shift) -> std::pair<T, bool>
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
inline auto lsrWithCarry(T x, int8_t shift) -> std::pair<T, bool>
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
inline auto asrWithCarry(T x, int8_t shift) -> std::pair<T, bool>
{
    assert(shift > 0);
    const auto carry = x & RIGHT_BIT<T>;
    if (x & LEFT_BIT<T>) {
        return {x >> shift | ~(ALL_BITS<T> >> shift), carry};
    }
    else {
        return {x >> shift, carry};
    }
}

template <typename T>
inline auto asr(T x, int8_t shift) -> T
{
    assert(shift >= 0);
    if (x & LEFT_BIT<T>) {
        return x >> shift | ~(ALL_BITS<T> >> shift);
    }
    else {
        return x >> shift;
    }
}

template <typename T>
inline auto rorWithCarry(T x, int8_t shift) -> std::pair<T, bool>
{
    assert(shift != 0);
    shift &= MAX_SHIFT<T>;
    const auto result = (x >> shift) | (x << (-shift & MAX_SHIFT<T>));
    return {result, result & LEFT_BIT<T>};
}

template <typename T>
inline auto ror(T x, int8_t shift) -> T
{
    if (shift == 0) {
        return x;
    }

    shift &= MAX_SHIFT<T>;
    return (x >> shift) | (x << (-shift & MAX_SHIFT<T>));
}

template <typename T>
inline auto rrxWithCarry(T x, bool carryIn) -> std::pair<T, bool>
{
    const auto carryOut = x & RIGHT_BIT<T>;
    return {(carryIn << MAX_SHIFT<T>) | (x >> 1u), carryOut};
}

template <typename T>
inline auto rrx(T x, bool carryIn) -> T
{
    return (carryIn << MAX_SHIFT<T>) | (x >> 1u);
}

enum class ShiftType {
    LSL,
    LSR,
    ASR,
    RRX,
    ROR,
};

inline auto decodeImmediateShift(uint8_t bits, uint8_t immediate) -> std::pair<ShiftType, uint8_t>
{
    switch (bits) {
        case 0b00u:
            return {ShiftType::LSL, immediate};
        case 0b01u:
            switch (immediate) {
                case 0b00000u:
                    return {ShiftType::LSR, 0b11111u};
                default:
                    return {ShiftType::LSR, immediate};
            }
        case 0b10u:
            switch (immediate) {
                case 0b00000u:
                    return {ShiftType::ASR, 0b11111u};
                default:
                    return {ShiftType::ASR, immediate};
            }
        case 0b11u:
            switch (immediate) {
                case 0b00000u:
                    return {ShiftType::RRX, 0b1u};
                default:
                    return {ShiftType::ROR, immediate};
            }
        default:
            UNPREDICTABLE;
    }
}

template <typename T>
inline auto shift(T value, ShiftType shiftType, int8_t shift, bool carryIn) -> T
{
    assert(shiftType != ShiftType::RRX || shift == 1u);

    if (shift == 0) {
        return value;
    }

    switch (shiftType) {
        case ShiftType::LSL:
            return lsl(value, shift);
        case ShiftType::LSR:
            return lsr(value, shift);
        case ShiftType::ASR:
            return asr(value, shift);
        case ShiftType::ROR:
            return ror(value, shift);
        case ShiftType::RRX:
            return rrx(value, carryIn);
        default:
            UNPREDICTABLE;
    }
}

template <typename T>
inline auto shiftWithCarry(T value, ShiftType shiftType, int8_t shift, bool carryIn) -> std::pair<T, bool>
{
    assert(shiftType != ShiftType::RRX || shift == 1u);

    if (shift == 0) {
        return {value, carryIn};
    }

    switch (shiftType) {
        case ShiftType::LSL:
            return lslWithCarry(value, shift);
        case ShiftType::LSR:
            return lsrWithCarry(value, shift);
        case ShiftType::ASR:
            return asrWithCarry(value, shift);
        case ShiftType::ROR:
            return rorWithCarry(value, shift);
        case ShiftType::RRX:
            return rrxWithCarry(value, carryIn);
        default:
            UNPREDICTABLE;
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

inline auto thumbExpandImmediateWithCarry(uint16_t immediate, bool carryIn) -> std::pair<uint32_t, bool>
{
    assert((immediate >> 12u) == 0u);

    if (math::getPart<10, 2>(immediate) == 0) {
        switch (math::getPart<8, 2>(immediate)) {
            case 0b00u:
                return {static_cast<uint32_t>(immediate), carryIn};
            case 0b01u: {
                const auto value = math::getPart<0, 8, uint8_t>(immediate);
                assert(value);
                return {combine<uint32_t>(Part<0, 8>{value}, Part<16, 8>{value}), carryIn};
            }
            case 0b10u: {
                const auto value = math::getPart<0, 8, uint8_t>(immediate);
                assert(value);
                return {combine<uint32_t>(Part<8, 8>{value}, Part<24, 8>{value}), carryIn};
            }
            case 0b11u: {
                const auto value = math::getPart<0, 8, uint8_t>(immediate);
                assert(value);
                return {combine<uint32_t>(Part<0, 8>{value}, Part<8, 8>{value}, Part<8, 16>{value}, Part<24, 8>{value}), carryIn};
            }
            default:
                UNPREDICTABLE;
        }
    }
    else {
        const auto unrotatedValue = static_cast<uint32_t>(getPart<0, 7>(immediate)) | BIT<7, uint32_t>;
        return rorWithCarry(unrotatedValue, getPart<7, 4>(immediate));
    }
}

}  // namespace stm32::math
