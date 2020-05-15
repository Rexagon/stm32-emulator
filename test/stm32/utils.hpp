#pragma once

#include <stm32/memory.hpp>

namespace details {
using namespace stm32;

auto createMemory() -> Memory
{
    return Memory{Memory::Config{
        .flashMemoryStart = 0x08000000u,
        .flashMemoryEnd = 0x0801FFFFu,

        .systemMemoryStart = 0x1FFFF000u,
        .systemMemoryEnd = 0x1FFFF800u,

        .optionBytesStart = 0x1FFFF800u,
        .optionBytesEnd = 0x1FFFF80Fu,

        .sramStart = 0x20000000u,
        .sramEnd = 0x20005000u,

        .bootMode = BootMode::FlashMemory,
    }};
}

auto createBitBandAddress(uint32_t address, uint8_t bitNumber, uint32_t bitBandAliasStart, uint32_t bitBandRegionStart) -> uint32_t
{
    const auto byteOffset = address - bitBandRegionStart;

    const auto bitWordOffset = static_cast<uint32_t>(byteOffset << 5u) + static_cast<uint32_t>(bitNumber << 2u);

    return bitWordOffset + bitBandAliasStart;
}

}  // namespace details
