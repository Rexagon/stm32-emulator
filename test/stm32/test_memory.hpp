#pragma once

#include <gtest/gtest.h>

#include <stm32/memory.hpp>

#include "utils.hpp"

TEST(memory, memory_consistency)
{
    auto memory = details::createMemory();

    const std::vector<std::pair<uint32_t, uint8_t>> testData = {
        {0x08000001u, 0xADu},  // flash consistency
        {0x1FFFF001u, 0xEAu},  // system memory consistency
        {0x1FFFF801u, 0x1Eu},  // option bytes consistency
        {0x20000001u, 0x57u},  // SRAM consistency
    };

    for (const auto& [address, data] : testData) {
        memory.write(address, data);
        ASSERT_EQ(memory.read(address), data);
    }
}

TEST(memory, reserved_addresses)
{
    auto memory = details::createMemory();

    const std::vector<std::pair<uint32_t, uint8_t>> testData = {
        {0x0801FFFFu, 0xFFu},  // flash memory end
        {0x09000000u, 0xFFu},  // reserved area in code region
        {0x1FFFF80Fu, 0xFFu},  // option bytes end
        {0x1FFFFFFFu, 0xFFu},  // end of reserved area in high bytes code region
        {0x29000000u, 0xFFu},  // sram region
        {0x20005000u, 0xFFu},  // sram end
    };

    for (const auto& [address, data] : testData) {
        memory.write(address, data);
        ASSERT_EQ(memory.read(address), 0);
    }
}

TEST(memory, bit_band_region_test)
{
    using namespace stm32;

    auto memory = details::createMemory();

    const std::vector<std::pair<uint32_t, uint8_t>> testData = {
        {0x20000000u, 0xADu},  //
        {0x20000100u, 0x55u},  //
        {0x20000101u, 0xFFu},  //
    };

    for (const auto& [address, data] : testData) {
        memory.write(address, data);

        for (uint8_t i = 0; i < 8u; ++i) {
            const auto bitValue = memory.read(details::createBitBandAddress(address, i, Memory::AddressSpace::SramBitBandAliasStart,
                                                                            Memory::AddressSpace::SramBitBandRegionStart));

            ASSERT_EQ(bitValue, static_cast<uint32_t>(data >> i) & 0x1u);
        }
    }
}

TEST(memory, bit_band_alias_test)
{
    using namespace stm32;

    using Data = std::pair<uint32_t, uint8_t>;
    using ByteData = std::pair<Data, std::vector<Data>>;

    auto memory = details::createMemory();

    const std::vector<ByteData> testData = {
        {
            {0x20000000u, 0b00001011u},  //
            {
                {0x22000000u, 0x03u},  // bit[0] = 1
                {0x22000004u, 0x01u},  // bit[1] = 1
                {0x22000008u, 0x00u},  // bit[2] = 0
                {0x2200000Cu, 0xFFu},  // bit[3] = 1
            }                          //
        },
        {
            {0x20000201u, 0b10100001u},  //
            {
                {0x22004020u, 0x03u},  // bit[0] = 1
                {0x22004034u, 0x01u},  // bit[5] = 1
                {0x2200403Cu, 0xFFu},  // bit[7] = 1
            }                          //
        },
        {
            {0x20000201u, 0b00000001u},  // repeat, but set some bits to zero
            {
                {0x22004034u, 0x0u},  // bit[5] = 0
                {0x2200403Cu, 0x0u},  // bit[7] = 0
            }                         //
        },
    };

    for (const auto& [targetData, bits] : testData) {
        for (const auto& [address, data] : bits) {
            memory.write(address, data);
        }

        const auto& [address, data] = targetData;
        ASSERT_EQ(memory.read(address), data);
    }
}
