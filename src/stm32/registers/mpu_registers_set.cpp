// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "mpu_registers_set.hpp"

#include <stm32/memory.hpp>

#include "../utils/math.hpp"

namespace stm32::rg
{
MpuRegistersSet::MpuRegistersSet()
    : m_mpuTypeRegister{}
    , m_mpuControlRegister{}
    , m_mpuRegionNumberRegister{}
    , m_mpuRegionBaseAddressRegisters{}
    , m_mpuRegionAttributeAndSizeRegisters{}
{
}

void MpuRegistersSet::reset()
{
    m_mpuTypeRegister.registerData = 0x00000800u;
    m_mpuControlRegister.registerData = 0u;
    m_mpuRegionNumberRegister.registerData = 0u;

    for (size_t i = 0; i < m_mpuRegionBaseAddressRegisters.size(); ++i) {
        m_mpuRegionBaseAddressRegisters[i].registerData = 0u;
        m_mpuRegionAttributeAndSizeRegisters[i].registerData = 0u;
    }
}

auto MpuRegistersSet::defaultTexDecode(const MpuRegionAttribute& attributes) -> MemoryAttributes
{
    using namespace utils;

    MemoryAttributes result;

    const auto texcb = combine<uint8_t>(Part<0, 1>{attributes.B}, Part<1, 1>{attributes.C}, Part<2, 3>{attributes.TEX});
    switch (texcb) {
        case 0b00000u:
            result.type = MemoryType::StronglyOrdered;
            result.inner = result.outer = CacheAttribute::NonCacheable;
            result.shareable = true;
            break;

        case 0b00001u:
            result.type = MemoryType::Device;
            result.inner = result.outer = CacheAttribute::NonCacheable;
            result.shareable = true;
            break;

        case 0b0001'0u ... 0b0001'1u:
        case 0b00100u:
            result.type = MemoryType::Normal;
            result.inner = result.outer = static_cast<CacheAttribute>(texcb & 0b11u);
            result.shareable = attributes.S;
            break;

        case 0b00110u:
            // TODO: implementation defined, check
            break;

        case 0b00111u:
            result.type = MemoryType::Normal;
            result.inner = result.outer = CacheAttribute::WBWA;
            result.shareable = attributes.S;
            break;

        case 0b01000u:
            result.type = MemoryType::Device;
            result.inner = result.outer = CacheAttribute::NonCacheable;
            result.shareable = false;
            break;

        case 0b1'0000u ... 0b1'1111u:
            result.type = MemoryType::Normal;
            result.inner = static_cast<CacheAttribute>(getPart<0, 2>(texcb));
            result.outer = static_cast<CacheAttribute>(getPart<2, 2>(texcb));
            result.shareable = attributes.S;
            break;

        default:
            UNPREDICTABLE;
    }

    return result;
}

}  // namespace stm32::rg
