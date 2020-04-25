#pragma once

#include <cstdint>
#include <vector>

namespace stm32 {
class MemoryRegion {
public:
    explicit MemoryRegion(uint32_t regionStart, uint32_t regionEnd);
    virtual ~MemoryRegion() = default;

    virtual void write(uint32_t address, uint8_t data) = 0;
    virtual auto read(uint32_t address) -> uint8_t = 0;

    inline auto regionStart() const -> uint32_t { return m_regionStart; }
    inline auto regionEnd() const -> uint32_t { return m_regionEnd; }

private:
    uint32_t m_regionStart;
    uint32_t m_regionEnd;
};

class Memory {
public:
    /**
     * @brief Memory regions ranges
     *
     * @par Memory Map
     *
     * Code:                    Instruction fetches are performed over the ICode bus. Data accesses are performed over
     *                          the DCode bus.
     *
     * SRAM:                    Instruction fetches and data accesses are performed over the system bus.
     *
     * SRAM bit-band:           Alias region. Data accesses are aliases. Instruction accesses are not aliases.
     *
     * Peripheral:              Instruction fetches and data accesses are performed over the system bus.
     *
     * Peripheral bit-band:     Alias region. Data accesses are aliases. Instruction accesses are not aliases.
     *
     * External RAM:            Instruction fetches and data accesses are performed over the system bus.
     *
     * External Device:         Instruction fetches and data accesses are performed over the system bus.
     *
     * Private Peripheral Bus:  External and internal Private Peripheral Bus (PPB) interfaces.
     *                          This memory region is Execute Never (XN), and so instruction fetches are prohibited. An
     *                          MPU, if present, cannot change this.
     *
     * System:                  System segment for vendor system peripherals. This memory region is XN, and so
     *                          instruction fetches are prohibited. An MPU, if present, cannot change this.
     *
     *
     * Internal Private Peripheral Bus (PPB) provides access to:
     * - The Instrumentation Trace Macrocell (ITM)
     * - The Data Watchpoint and Trace (DWT)
     * - The Flashpatch and Breakpoint (FPB)
     * - The System Control Space (SCS), including the Memory Protection Unit (MPU) and the
     *   Nested Vectored Interrupt Controller (NVIC)
     *
     *
     * External Private Peripheral Bus (PPB) provides access to:
     * - The Trace Point Interface Unit (TPIU)
     * - The Embedded Trace Macrocell (ETM)
     * - The ROM table
     * - Implementation-specific areas of the PPB memory map
     *
     */
    enum AddressSpace : uint32_t {
        CodeStart = 0x00000000u,
        CodeEnd = 0x20000000u,

        SramStart = 0x20000000u,
        SramEnd = 0x40000000u,

        SramBitBandRegionStart = 0x20000000u,
        SramBitBandRegionEnd = 0x20100000u,
        SramBitBandAliasStart = 0x22000000u,
        SramBitBandAliasEnd = 0x24000000u,

        PeripheralStart = 0x40000000u,
        PeripheralEnd = 0x60000000u,

        PeripheralBitBandRegionStart = 0x40000000u,
        PeripheralBitBandRegionEnd = 0x40100000u,
        PeripheralBitBandAliasStart = 0x42000000u,
        PeripheralBitBandAliasEnd = 0x44000000u,

        ExternalRamStart = 0x60000000u,
        ExternalRamEnd = 0xA0000000u,

        ExternalDeviceStart = 0xA0000000u,
        ExternalDeviceEnd = 0xE0000000u,

        InternalPrivatePeripheralBusStart = 0xE0000000u,
        InternalPrivatePeripheralBusEnd = 0xE0040000u,

        ItmStart = 0xE0000000u,
        ItmEnd = 0xE0001000u,
        DwtStart = 0xE0001000u,
        DwtEnd = 0xE0002000u,
        FpbStart = 0xE0002000u,
        FpbEnd = 0xE0003000u,
        ScsStart = 0xE000E000u,
        ScsEnd = 0xE000F000u,

        ExternalPrivatePeripherialBusStart = 0xE0040000u,
        ExternalPrivatePeripherialBusEnd = 0xE0100000u,

        TpiuStart = 0xE0040000,
        TpiuEnd = 0xE0041000u,
        EtmStart = 0xE0041000u,
        EtmEnd = 0xE0042000u,
        ExternalPpbStart = 0xE0042000u,
        ExternalPpbEnd = 0xE00FF000u,
        RomTableStart = 0xE00FF000u,
        RomTableEnd = 0xE0100000u,

        SystemStart = 0xE0100000u,
        SystemEnd = 0xFFFFFFFF
    };

    struct Config {
        uint32_t flashMemoryStart;
        uint32_t flashMemoryEnd;

        uint32_t systemMemoryStart;
        uint32_t systemMemoryEnd;

        uint32_t optionBytesStart;
        uint32_t optionBytesEnd;

        uint32_t sramStart;
        uint32_t sramEnd;
    };

    explicit Memory(const Config& config);

    void attachRegion(MemoryRegion& region);

    void write(uint32_t address, uint8_t data);
    auto read(uint32_t address) const -> uint8_t;

    inline auto config() const -> const Config& { return m_config; }

private:
    auto findRegion(uint32_t address) const -> MemoryRegion*;

    Config m_config;

    std::vector<uint8_t> m_flash;
    std::vector<uint8_t> m_systemMemory;
    std::vector<uint8_t> m_optionBytes;

    std::vector<uint8_t> m_sram;

    std::vector<MemoryRegion*> m_memoryRegions;
};

}  // namespace stm32
