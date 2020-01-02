#pragma once

#include <cstdint>
#include <vector>

namespace stm32
{
class SystemBus
{
public:
    enum AddressSpace : uint32_t
    {
        CodeStart = 0x00000000u,
        CodeEnd = 0x20000000u,

        SramStart = 0x20000000u,
        SramEnd = 0x40000000u,

        PeripheralStart = 0x40000000u,
        PeripheralEnd = 0x60000000u,

        BitBandRegionStart = 0x40000000u,
        BitBandRegionEnd = 0x40100000u,
        BitBandAliasStart = 0x42000000u,
        BitBandAliasEnd = 0x44000000u,

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

    void write(uint32_t address, uint8_t data);
    uint8_t read(uint32_t address) const;

private:
};

}  // namespace stm32
