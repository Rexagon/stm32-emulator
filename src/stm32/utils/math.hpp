#pragma once

#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <tuple>
#include <utility>

#include "../utils/general.hpp"

namespace stm32::utils
{
// Lookup table that store the reverse of each table
constexpr uint8_t REVERSE_BITS_LUT[16] = {
    0x0,
    0x8,
    0x4,
    0xc,
    0x2,
    0xa,
    0x6,
    0xe,
    0x1,
    0x9,
    0x5,
    0xd,
    0x3,
    0xb,
    0x7,
    0xf,
};

template <typename T, typename R = T>
constexpr R MAX_SHIFT = static_cast<R>(sizeof(T) * 8u - 1u);

template <typename T, typename R = T>
constexpr R LEFT_BIT = static_cast<R>(T{0b1u} << MAX_SHIFT<T>);

template <uint8_t number, typename T>
constexpr T BIT = static_cast<T>(T{0b1u} << number);

template <typename T>
inline constexpr auto makeBit(uint8_t number) -> T
{
    return static_cast<T>(T{0b1u} << number);
}

template <typename T>
constexpr T ALL_BITS = ~T{0b0u};

template <uint8_t N, typename T>
constexpr T ONES = N > 31 ? std::numeric_limits<T>::max() : (T{0b1u} << N) - T{0b1u};

template <typename T>
inline constexpr auto makeOnes(uint8_t count) -> T
{
    return static_cast<T>((T{0b1u} << count) - T{0b1u});
}

template <uint8_t N, typename T>
constexpr T ZEROS = static_cast<T>(~ONES<N, T>);

template <typename T>
inline constexpr auto makeZeros(uint8_t count) -> T
{
    return static_cast<T>(~makeOnes<T>(count));
}

template <uint8_t _offset, uint8_t _bitCount, typename T = uint8_t>
struct Part {
    static_assert((sizeof(T) * 8u) >= _bitCount);
    T value;

    constexpr static auto offset = _offset;
    constexpr static auto bitCount = _bitCount;
};

template <uint8_t offset, uint8_t bitCount = 1u, typename T = uint8_t>
using _ = Part<offset, bitCount, T>;

namespace extractor
{
template <typename, typename>
struct Extractor;

template <typename V, uint8_t offset, uint8_t bitCount, typename T>
struct Extractor<V, Part<offset, bitCount, T>> {
    using Result = T;
    static constexpr auto create(const T& part) -> V { return static_cast<V>((static_cast<V>(part) & ONES<bitCount, V>) << offset); }
    static constexpr auto extract(const V& value) -> T { return static_cast<T>(value >> offset) & ONES<bitCount, T>; }
};
}  // namespace extractor

template <typename... Parts, typename V>
inline constexpr auto split(const V& value) -> std::tuple<typename extractor::Extractor<V, Parts>::Result...>
{
    return std::tuple(extractor::Extractor<V, Parts>::extract(value)...);
}

template <typename V, typename... Parts>
inline constexpr auto combine(Parts... part) -> V
{
    return (extractor::Extractor<V, Parts>::create(part.value) | ...);
}

template <uint8_t offset, uint8_t bitCount, typename R = uint8_t, typename V>
inline constexpr auto getPart(const V& value) -> R
{
    static_assert((sizeof(R) * 8u) >= bitCount);
    return static_cast<R>(static_cast<V>(value >> offset) & ONES<bitCount, V>);
}

template <typename R = uint8_t, typename V>
constexpr auto getPart(const V& value, uint8_t offset, uint8_t bitCount) -> R
{
    return static_cast<R>(static_cast<V>(value >> offset) & ((V{0b1u} << bitCount) - V{0b1u}));
}

template <uint8_t number, typename V>
inline constexpr auto isBitSet(const V& value) -> bool
{
    return value & BIT<number, V>;
}

template <uint8_t number, typename V>
inline constexpr auto isBitClear(const V& value) -> bool
{
    return !isBitSet<number>(value);
}

template <typename V>
inline auto isBitSet(const V& value, uint8_t number) -> bool
{
    return value & static_cast<V>(V{1u} << number);
}

template <typename V>
inline auto isBitClear(const V& value, uint8_t number) -> bool
{
    return !isBitSet(value, number);
}

template <typename T>
inline constexpr auto bitCount(const T& value) -> uint32_t
{
#ifdef __GNUC__
    return static_cast<uint32_t>(__builtin_popcount(value));
#else
    auto n = value;
    size_t count{};
    while (n) {
        n &= (n - 1u);
        count++;
    }
    return count;
#endif
}

template <typename T>
inline constexpr auto lowestSetBit(const T& value) -> uint32_t
{
#ifdef __GNUC__
    return value == 0u ? sizeof(T) * 8u : static_cast<uint32_t>(__builtin_ffs(value));
#else
    static_assert(false);
#endif
}

template <typename T>
inline constexpr auto countLeadingZeros(const T& value) -> uint32_t
{
#ifdef __GNUC__
    return value == 0u ? sizeof(T) * 8u : static_cast<uint32_t>(__builtin_clz(value));
#else
    static_assert(false);
#endif
}

template <typename T>
inline constexpr auto isNegative(const T& value) -> bool
{
    return value & LEFT_BIT<T>;
}

inline constexpr auto reverseEndianness(const uint8_t& value) -> uint8_t
{
    return value;
}

inline constexpr auto reverseEndianness(const uint16_t& value) -> uint16_t
{
#ifdef __GNUC__
    return __builtin_bswap16(value);
#else
    const auto [lo, hi] = split<_<0, 8>, _<8, 8>>(value);
    return combite<uint16_t>(_<0, 8>{hi}, _<8, 8>{lo});
#endif
}

inline constexpr auto reverseEndianness(const uint32_t& value) -> uint32_t
{
#ifdef __GNUC__
    return __builtin_bswap32(value);
#else
    const auto [hw1lo, hw1hi, hw2lo, hw2hi] = split<_<0, 8>, _<8, 8>, _<16, 8>, _<24, 8>>(value);
    return combite<uint16_t>(_<0, 8>{hw2hi}, _<8, 8>{hw2lo}, _<16, 8>{hw1hi}, _<24, 8>{hw1lo});
#endif
}

template <typename T>
inline constexpr auto reverseBits(const T& value) -> T
{
    if constexpr (std::is_same_v<T, uint8_t>) {
        const auto [low, high] = split<_<0, 4>, _<4, 4>>(value);
        return combine<uint8_t>(_<0, 4>{REVERSE_BITS_LUT[high]}, _<4, 4>{REVERSE_BITS_LUT[low]});
    }
    else if constexpr (std::is_same_v<T, uint16_t>) {
        const auto [low, high] = split<_<0, 8>, _<8, 8>>(value);
        return combine<uint16_t>(_<0, 8>{reverseBits<uint8_t>(high)}, _<8, 8>{reverseBits<uint8_t>(low)});
    }
    else if constexpr (std::is_same_v<T, uint32_t>) {
        const auto [hw1low, hw1high, hw2low, hw2high] = split<_<0, 8>, _<8, 8>, _<16, 8>, _<24, 8>>(value);
        return combine<uint32_t>(_<0, 8>{reverseBits<uint8_t>(hw2high)},
                                 _<8, 8>{reverseBits<uint8_t>(hw2low)},
                                 _<16, 8>{reverseBits<uint8_t>(hw1high)},
                                 _<24, 8>{reverseBits<uint8_t>(hw1low)});
    }
}

template <uint8_t bits, typename T>
constexpr auto signExtend(const T& value) -> uint32_t
{
    static_assert(bits > 0);
    const auto maskedSign = static_cast<T>(value & ZEROS<bits - 1, T>);
    return static_cast<T>(~static_cast<T>(maskedSign - 1u) & (~maskedSign | maskedSign)) | value;
}

template <typename T>
constexpr auto signExtend(const T& value, uint8_t bits)
{
    assert(bits > 0);
    const auto maskedSign = static_cast<T>(value & makeZeros<T>(static_cast<uint8_t>(bits - 1u)));
    return static_cast<T>(~static_cast<T>(maskedSign - 1u) & (~maskedSign | maskedSign)) | value;
}

template <typename T>
constexpr auto signedSaturateQ(const T& value, uint8_t n) -> std::pair<T, bool>
{
    assert(n > 0);
    const auto target = static_cast<T>(T{0b1} << n) - T{1u};
    if (value > target) {
        return {target, true};
    }
    else if (value < -target) {
        return {-target, true};
    }
    return {value, false};
}

template <typename T>
constexpr auto unsignedSaturateQ(const T& value, uint8_t n) -> std::pair<T, bool>
{
    assert(n > 0);
    const auto target = static_cast<T>(T{0b1} << n) - T{1u};
    if (value > target) {
        return {target, true};
    }
    else if (value < 0) {
        return {0u, true};
    }
    return {value, false};
}

template <typename T>
constexpr auto signedSaturate(const T& value, uint8_t n) -> T
{
    assert(n > 0);
    const auto target = static_cast<T>(T{0b1} << n) - T{1u};
    if (value > target) {
        return target;
    }
    else if (value < -target) {
        return -target;
    }
    return value;
}

template <typename T>
constexpr auto unsignedSaturate(const T& value, uint8_t n) -> T
{
    assert(n > 0);
    const auto target = static_cast<T>(T{0b1} << n) - T{1u};
    if (value > target) {
        return target;
    }
    else if (value < 0) {
        return 0u;
    }
    return value;
}

template <uint8_t lsb, uint8_t msb, typename T>
constexpr auto clearBitField(const T& value) -> T
{
    return value & (ONES<lsb, T> | ZEROS<msb + 1u, T>);
}

template <typename T>
constexpr auto clearBitField(const T& value, uint8_t lsb, uint8_t msb) -> T
{
    return value & (makeOnes<T>(lsb) | makeZeros<T>(static_cast<uint8_t>(msb + 1u)));
}

template <typename P1, typename P2, typename T>
constexpr auto copyPartInto(const T& source, const T& target) -> T
{
    static_assert(P1::bitCount == P2::bitCount);
    return clearBitField<P2::offset, P2::offset + P2::bitCount - 1u>(target) |
           static_cast<T>(getPart<P1::offset, P1::bitCount>(source) << P2::offset);
}

template <typename T>
inline auto lslWithCarry(T x, uint8_t shift) -> std::pair<T, bool>
{
    assert(shift > 0);
    const auto carry = x & LEFT_BIT<T>;
    return {x << shift, carry};
}

template <typename T>
inline auto lsl(T x, uint8_t shift) -> T
{
    return x << shift;
}

template <typename T>
inline auto lsrWithCarry(T x, uint8_t shift) -> std::pair<T, bool>
{
    assert(shift > 0);
    const auto carry = isBitSet<0>(x);
    return {x >> shift, carry};
}

template <typename T>
inline auto lsr(T x, uint8_t shift) -> T
{
    return x >> shift;
}

template <typename T>
inline auto asrWithCarry(T x, uint8_t shift) -> std::pair<T, bool>
{
    assert(shift > 0);
    const auto carry = isBitSet<0>(x);
    if (x & LEFT_BIT<T>) {
        return {x >> shift | ~(ALL_BITS<T> >> shift), carry};
    }
    else {
        return {x >> shift, carry};
    }
}

template <typename T>
inline auto asr(T x, uint8_t shift) -> T
{
    if (x & LEFT_BIT<T>) {
        return x >> shift | ~(ALL_BITS<T> >> shift);
    }
    else {
        return x >> shift;
    }
}

template <typename T>
inline auto rorWithCarry(T x, uint8_t shift) -> std::pair<T, bool>
{
    assert(shift > 0);

    const auto invShift = static_cast<uint8_t>(~shift) + 1u;
    const auto result = (x >> (shift & MAX_SHIFT<T>)) | (x << (invShift & MAX_SHIFT<T>));
    return {result, result & LEFT_BIT<T>};
}

template <typename T>
inline auto ror(T x, uint8_t shift) -> T
{
    if (shift == 0) {
        return x;
    }

    const auto invShift = static_cast<uint8_t>(~shift) + 1u;
    return (x >> (shift & MAX_SHIFT<T>)) | (x << (invShift & MAX_SHIFT<T>));
}

template <typename T>
inline auto rrxWithCarry(T x, bool carryIn) -> std::pair<T, bool>
{
    const auto carryOut = isBitSet<0>(x);
    return {static_cast<T>(carryIn << MAX_SHIFT<T>) | (x >> 1u), carryOut};
}

template <typename T>
inline auto rrx(T x, bool carryIn) -> T
{
    return static_cast<T>(carryIn << MAX_SHIFT<T>) | (x >> 1u);
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
inline auto shift(T value, ShiftType shiftType, uint8_t shift, bool carryIn) -> T
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
inline auto shiftWithCarry(T value, ShiftType shiftType, uint8_t shift, bool carryIn) -> std::pair<T, bool>
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

    if (getPart<10, 2>(immediate) == 0) {
        switch (getPart<8, 2>(immediate)) {
            case 0b00u:
                return {static_cast<uint32_t>(immediate), carryIn};
            case 0b01u: {
                const auto value = getPart<0, 8>(static_cast<uint8_t>(immediate));
                assert(value);
                return {combine<uint32_t>(_<0, 8>{value}, _<16, 8>{value}), carryIn};
            }
            case 0b10u: {
                const auto value = getPart<0, 8>(static_cast<uint8_t>(immediate));
                assert(value);
                return {combine<uint32_t>(_<8, 8>{value}, _<24, 8>{value}), carryIn};
            }
            case 0b11u: {
                const auto value = getPart<0, 8>(static_cast<uint8_t>(immediate));
                assert(value);
                return {combine<uint32_t>(_<0, 8>{value}, _<8, 8>{value}, _<16, 8>{value}, _<24, 8>{value}), carryIn};
            }
            default:
                UNPREDICTABLE;
        }
    }
    else {
        const auto unrotatedValue = static_cast<uint32_t>(getPart<0, 7>(immediate)) | BIT<7, uint32_t>;
        return rorWithCarry(unrotatedValue, getPart<7, 5>(immediate));
    }
}

template <typename T>
constexpr auto getAlignmentBitCount() -> uint32_t
{
    static_assert(std::is_same_v<T, uint32_t> || std::is_same_v<T, uint16_t> || std::is_same_v<T, uint8_t>);

    if constexpr (std::is_same_v<T, uint32_t>) {
        return 2u;
    }
    else if constexpr (std::is_same_v<T, uint16_t>) {
        return 1u;
    }
    else if constexpr (std::is_same_v<T, uint8_t>) {
        return 2u;
    }
}

template <typename T>
auto alignAddress(uint32_t address) -> T
{
    return address & ZEROS<getAlignmentBitCount<T>(), uint32_t>;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-parameter"
template <typename T>
auto isAddressAligned(uint32_t address) -> bool
{
    if constexpr (std::is_same_v<T, uint8_t>) {
        return true;
    }
    else if constexpr (std::is_same_v<T, uint16_t>) {
        return (address & ZEROS<1, uint32_t>) == 0;
    }
    else if constexpr (std::is_same_v<T, uint32_t>) {
        return (address & ZEROS<2, uint32_t>) == 0;
    }
}
#pragma GCC diagnostic pop

}  // namespace stm32::utils
