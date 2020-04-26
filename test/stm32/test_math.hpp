#pragma once

#include <gtest/gtest.h>

#include <stm32/math.hpp>

TEST(math, ONES)
{
    using namespace stm32::math;

    ASSERT_EQ((ONES<4, uint8_t>), 0b1111u);
}

TEST(math, parts)
{
    using namespace stm32::math;

    const auto [first, second] = split<uint16_t, Part<0, 8>, Part<4, 5>>(0x1f22u);
    ASSERT_EQ(first, 0x22u);
    ASSERT_EQ(second, 0x12u);

    ASSERT_EQ((combine<uint16_t>(Part<2, 8>{first}, Part<10, 5>{second})), 0x4888u);

    ASSERT_EQ((getPart<4, 5>(0x0ff0u)), 0x1fu);
}

TEST(math, LSL)
{
    using namespace stm32::math;

    ASSERT_EQ(lsl(0x00000001u, 1), 0x00000002u);
    ASSERT_EQ(lsl(0xf0000001u, 1), 0xE0000002u);

    ASSERT_EQ(lslWithCarry(0x00000001u, 1), (std::pair{0x00000002u, false}));
    ASSERT_EQ(lslWithCarry(0xf0000001u, 1), (std::pair{0xE0000002u, true}));
}

TEST(math, LSR)
{
    using namespace stm32::math;

    ASSERT_EQ(lsr(0x00000002u, 1), 0x00000001u);
    ASSERT_EQ(lsr(0x80000002u, 1), 0x40000001u);

    ASSERT_EQ(lsrWithCarry(0x00000001u, 1), (std::pair{0x00000000u, true}));
    ASSERT_EQ(lsrWithCarry(0x80000002u, 1), (std::pair{0x40000001u, false}));
}

TEST(math, ASR)
{
    using namespace stm32::math;

    ASSERT_EQ(asr(0x80000002u, 1), 0xC0000001u);
    ASSERT_EQ(asr(0x80000002u, 1), 0xC0000001u);

    ASSERT_EQ(asrWithCarry(0x00000001u, 1), (std::pair{0x00000000u, true}));
    ASSERT_EQ(asrWithCarry(0x80000002u, 1), (std::pair{0xC0000001u, false}));
}

TEST(math, ROR)
{
    using namespace stm32::math;

    ASSERT_EQ(ror(0x00000003u, 1), 0x80000001u);
    ASSERT_EQ(ror(0x80000003u, 1), 0xC0000001u);

    ASSERT_EQ(rorWithCarry(0x00000003u, 1), (std::pair{0x80000001u, true}));
    ASSERT_EQ(rorWithCarry(0x00000003u, 2), (std::pair{0xC0000000u, true}));
    ASSERT_EQ(rorWithCarry(0x00000103u, 4), (std::pair{0x30000010u, false}));
    ASSERT_EQ(rorWithCarry(0x00000103u, 8), (std::pair{0x03000001u, false}));

    ASSERT_EQ(rorWithCarry(0x00000002u, 1), (std::pair{0x00000001u, false}));
}

TEST(math, RRX)
{
    using namespace stm32::math;

    ASSERT_EQ(rrx(0x00000003u, true), 0x80000001u);
    ASSERT_EQ(rrx(0x00000003u, false), 0x00000001u);

    ASSERT_EQ(rrxWithCarry(0x00000003u, true), (std::pair{0x80000001u, true}));
    ASSERT_EQ(rrxWithCarry(0x00000002u, false), (std::pair{0x00000001u, false}));
}

TEST(math, ADC)
{
    using namespace stm32::math;

    const auto neg = []<typename T>(T value) -> T { return static_cast<T>(~value) + 1u; };

    ASSERT_EQ(addWithCarry(0x00000001u, 0x00000002u, false), (std::tuple{0x00000003u, false, false}));
    ASSERT_EQ(addWithCarry(0xfffffff0u, 0x000000ffu, false), (std::tuple{0x000000efu, true, false}));

    ASSERT_EQ(addWithCarry(neg(0x3u), neg(0xAu), false), (std::tuple{neg(0xDu), true, false}));
    ASSERT_EQ(addWithCarry(0x3u, neg(0xAu), false), (std::tuple{neg(0x7u), false, false}));

    ASSERT_EQ(addWithCarry(0x7fffffffu, 0x7fffffffu, false), (std::tuple{neg(2u), false, true}));
    ASSERT_EQ(addWithCarry(neg(0x7fffffffu), neg(0x7fffffffu), false), (std::tuple{0x2u, true, true}));
}

TEST(math, thumbExpandImmediateWithCarry)
{
    using namespace stm32::math;

    const auto pack = [](uint8_t high, uint8_t mid, uint8_t low) -> uint16_t {
        return combine<uint16_t>(Part<0, 8>{low}, Part<8, 2>{mid}, Part<10, 2>{high});
    };

    ASSERT_EQ(pack(0b01u, 0b00, 0x7fu), 0x47f);
    ASSERT_EQ((getPart<7, 5, uint16_t, uint8_t>(0x47f)), 0x8u);
    ASSERT_EQ((rorWithCarry(0x000000ffu, 0x8u)), (std::pair{0xff000000u, true}));

    ASSERT_EQ(thumbExpandImmediateWithCarry(pack(0, 0b00, 0xeeu), false), (std::pair{0x000000eeu, false}));
    ASSERT_EQ(thumbExpandImmediateWithCarry(pack(0, 0b01, 0xeeu), false), (std::pair{0x00ee00eeu, false}));
    ASSERT_EQ(thumbExpandImmediateWithCarry(pack(0, 0b10, 0xeeu), false), (std::pair{0xee00ee00u, false}));
    ASSERT_EQ(thumbExpandImmediateWithCarry(pack(0, 0b11, 0xeeu), false), (std::pair{0xeeeeeeeeu, false}));

    ASSERT_EQ(thumbExpandImmediateWithCarry(pack(0b01u, 0b00, 0x7fu), false), (std::pair{0xff000000u, true}));
}