#pragma once

#include <gtest/gtest.h>

#include <stm32/memory.hpp>

namespace details
{
using namespace stm32;

Memory createMemory()
{
    return Memory{Memory::Config{
        0x08000000u,  // FLASH memory start
        0x0801FFFFu,  // FLASH memory end

        0x1FFFF000u,  // System memory start
        0x1FFFF800u,  // System memory end

        0x1FFFF800u,  // Option bytes start
        0x1FFFF80Fu,  // Option bytes end

        0x20000000u,  // SRAM start
        0x20005000u,  // SRAM end
    }};
}

uint32_t createBitBandAddress(uint32_t address,
                              uint8_t bitNumber,
                              uint32_t bitBandAliasStart,
                              uint32_t bitBandRegionStart)
{
    const auto byteOffset = address - bitBandRegionStart;

    const auto bitWordOffset = static_cast<uint32_t>(byteOffset << 5u) + static_cast<uint32_t>(bitNumber << 2u);

    return bitWordOffset + bitBandAliasStart;
}

}  // namespace details

TEST(memory, memory_consistency)
{
    auto memory = details::createMemory();

    const std::vector<std::pair<uint32_t, uint8_t>> testData = {
        {0x08000001u, 0xADu},  // flash consistency
        {0x1FFFF001u, 0xEAu},  // system memory consistency
        {0x1FFFF801u, 0x1Eu},  // option bytes consistency
        {0x20000001u, 0x57u},  // SRAM consistency
    };

    for (const auto &[address, data] : testData)
    {
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

    for (const auto &[address, data] : testData)
    {
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

    for (const auto &[address, data] : testData)
    {
        memory.write(address, data);

        for (uint8_t i = 0; i < 8u; ++i)
        {
            const auto bitValue = memory.read(details::createBitBandAddress(
                address, i, Memory::AddressSpace::SramBitBandAliasStart, Memory::AddressSpace::SramBitBandRegionStart));

            ASSERT_EQ(bitValue, static_cast<uint32_t>(data >> i) & 0x1u);
        }
    }
}
