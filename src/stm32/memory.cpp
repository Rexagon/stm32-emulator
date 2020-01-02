// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "memory.hpp"

namespace stm32
{
namespace
{
inline std::pair<uint32_t, uint8_t> decodeBitBand(uint32_t address,
                                                  uint32_t bitBandAliasStart,
                                                  uint32_t bitBandRegionStart)
{
    // bit_word_offset = (byte_offset * 32) + (bit_number * 4)
    // bit_word_address = bit_band_offset + bit_word_offset

    const auto bitAddress = address - bitBandAliasStart;
    const auto bitNumber = bitAddress >> 2u;
    const auto referencedByte = bitAddress >> 5u;

    return {bitBandRegionStart + referencedByte, bitNumber};
}

inline uint8_t setBit(uint8_t data, uint8_t bitNumber, uint8_t value)
{
    // Clear nth bit and set it same as the lowest data bit
    return (data & ~(0x1u << bitNumber)) | ((value & 0x1u) << bitNumber);
}

}  // namespace

MemoryRegion::MemoryRegion(uint32_t regionStart, uint32_t regionEnd)
    : m_regionStart{regionStart}
    , m_regionEnd{regionEnd}
{
}


uint32_t MemoryRegion::regionStart() const
{
    return m_regionStart;
}


uint32_t MemoryRegion::regionEnd() const
{
    return m_regionEnd;
}


Memory::Memory(const Config &config)
    : m_config{config}
    , m_flash(config.flashMemoryEnd - config.flashMemoryStart, 0)
    , m_systemMemory(config.systemMemoryEnd - config.systemMemoryStart, 0)
    , m_optionBytes(config.optionBytesEnd - config.optionBytesStart, 0)
    , m_sram(config.sramEnd - config.sramStart, 0)
{
}


void Memory::attachRegion(MemoryRegion &region)
{
    auto it = m_memoryRegions.begin();
    for (; it != m_memoryRegions.end() && (*it)->regionEnd() < region.regionStart(); ++it)
    {
        // skip until inserted region range is greater than current
    }

    m_memoryRegions.insert(it, &region);
}

void Memory::write(uint32_t address, uint8_t data)
{
    if (address < m_config.flashMemoryEnd)
    {
        if (address < m_config.flashMemoryStart)
        {
            // TODO: handle write to aliased memory depending on BOOT pins
        }
        else
        {
            m_flash[address - m_config.flashMemoryStart] = data;
        }
    }
    else if (address >= m_config.systemMemoryStart && address < m_config.systemMemoryEnd)
    {
        m_systemMemory[address - m_config.systemMemoryStart] = data;
    }
    else if (address >= m_config.optionBytesStart && address < m_config.optionBytesEnd)
    {
        m_optionBytes[address - m_config.optionBytesStart] = data;
    }
    else if (address >= m_config.sramStart && address < m_config.sramEnd)
    {
        m_sram[address - m_config.sramStart] = data;
    }
    else if (address >= AddressSpace::SramBitBandAliasStart && address < AddressSpace::SramBitBandAliasEnd)
    {
        const auto [referencedAddress, bitNumber] =
            decodeBitBand(address, AddressSpace::SramBitBandAliasStart, AddressSpace::SramBitBandRegionStart);

        if (referencedAddress >= m_config.sramStart && referencedAddress < m_config.sramEnd)
        {
            auto &sramCell = m_sram[referencedAddress];
            sramCell = setBit(sramCell, bitNumber, data);
        }
    }
    else if (address >= AddressSpace::PeripheralBitBandAliasStart && address < AddressSpace::PeripheralBitBandAliasEnd)
    {
        const auto [referencedAddress, bitNumber] = decodeBitBand(address, AddressSpace::PeripheralBitBandAliasStart,
                                                                  AddressSpace::PeripheralBitBandRegionStart);

        if (auto *region = findRegion(referencedAddress); region != nullptr)
        {
            region->write(referencedAddress, setBit(region->read(referencedAddress), bitNumber, data));
        }
    }
    else if (auto *region = findRegion(address); region != nullptr)
    {
        region->write(address, data);
    }
}


uint8_t Memory::read(uint32_t address) const
{
    if (address < m_config.flashMemoryEnd)
    {
        if (address < m_config.flashMemoryStart)
        {
            // TODO: handle write to aliased memory depending on BOOT pins
        }
        else
        {
            return m_flash[address - m_config.flashMemoryStart];
        }
    }
    else if (address >= m_config.systemMemoryStart && address < m_config.systemMemoryEnd)
    {
        return m_systemMemory[address - m_config.systemMemoryStart];
    }
    else if (address >= m_config.optionBytesStart && address < m_config.optionBytesEnd)
    {
        return m_optionBytes[address - m_config.optionBytesStart];
    }
    else if (address >= m_config.sramStart && address < m_config.sramEnd)
    {
        return m_sram[address - m_config.sramStart];
    }
    else if (address >= AddressSpace::SramBitBandAliasStart && address < AddressSpace::SramBitBandAliasEnd)
    {
        const auto [referencedAddress, bitNumber] =
            decodeBitBand(address, AddressSpace::SramBitBandAliasStart, AddressSpace::SramBitBandRegionStart);

        return static_cast<uint8_t>(m_sram[referencedAddress] >> bitNumber) & 0x1u;
    }
    else if (address >= AddressSpace::PeripheralBitBandAliasStart && address < AddressSpace::PeripheralBitBandAliasEnd)
    {
        const auto [referencedAddress, bitNumber] = decodeBitBand(address, AddressSpace::PeripheralBitBandAliasStart,
                                                                  AddressSpace::PeripheralBitBandRegionStart);

        if (auto *region = findRegion(referencedAddress); region != nullptr)
        {
            return static_cast<uint8_t>(region->read(referencedAddress) >> bitNumber) & 0x1u;
        }
    }
    else if (auto *region = findRegion(address); region != nullptr)
    {
        return region->read(address);
    }

    return 0;
}


MemoryRegion *Memory::findRegion(uint32_t address) const
{
    if (m_memoryRegions.empty())
    {
        return nullptr;
    }

    size_t position = m_memoryRegions.size() >> 1u;
    size_t stepSize = position >> 1u;

    while (true)
    {
        if (stepSize == 0)
        {
            return nullptr;
        }

        if (m_memoryRegions[position]->regionEnd() < address)
        {
            position += stepSize;
        }
        else if (m_memoryRegions[position]->regionStart() > address)
        {
            position -= stepSize;
        }
        else
        {
            return m_memoryRegions[position];
        }

        stepSize >>= 1u;
    }
}
}  // namespace stm32
