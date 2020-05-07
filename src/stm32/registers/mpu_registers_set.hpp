#pragma once

#include <array>

#include "mpu_registers.hpp"
#include "../memory.hpp"

namespace stm32::rg
{
class MpuRegistersSet {
public:
    explicit MpuRegistersSet();

    void reset();

    inline auto MPU_TYPE() const -> const MpuTypeRegister& { return m_mpuTypeRegister; }

    inline auto MPU_CTRL() -> MpuControlRegister& { return m_mpuControlRegister; }
    inline auto MPU_CTRL() const -> const MpuControlRegister& { return m_mpuControlRegister; }

    inline auto MPU_RNR() -> MpuRegionNumberRegister& { return m_mpuRegionNumberRegister; }
    inline auto MPU_RNR() const -> const MpuRegionNumberRegister& { return m_mpuRegionNumberRegister; }

    inline auto MPU_RBAR() -> MpuRegionBaseAddressRegister&
    {
        assert(m_mpuRegionNumberRegister.REGION < m_mpuRegionBaseAddressRegisters.size());
        return m_mpuRegionBaseAddressRegisters[m_mpuRegionNumberRegister.REGION];
    }
    inline auto MPU_RBAR() const -> const MpuRegionBaseAddressRegister&
    {
        assert(m_mpuRegionNumberRegister.REGION < m_mpuRegionBaseAddressRegisters.size());
        return m_mpuRegionBaseAddressRegisters[m_mpuRegionNumberRegister.REGION];
    }
    inline auto MPU_RBAR(uint8_t region) -> MpuRegionBaseAddressRegister&
    {
        assert(region < m_mpuRegionBaseAddressRegisters.size());
        return m_mpuRegionBaseAddressRegisters[region];
    }
    inline auto MPU_RBAR(uint8_t region) const -> const MpuRegionBaseAddressRegister&
    {
        assert(region < m_mpuRegionBaseAddressRegisters.size());
        return m_mpuRegionBaseAddressRegisters[region];
    }

    inline auto MPU_RASR() -> MpuRegionAttributeAndSizeRegister&
    {
        assert(m_mpuRegionNumberRegister.REGION < m_mpuRegionAttributeAndSizeRegisters.size());
        return m_mpuRegionAttributeAndSizeRegisters[m_mpuRegionNumberRegister.REGION];
    }

    inline auto MPU_RASR() const -> const MpuRegionAttributeAndSizeRegister&
    {
        assert(m_mpuRegionNumberRegister.REGION < m_mpuRegionAttributeAndSizeRegisters.size());
        return m_mpuRegionAttributeAndSizeRegisters[m_mpuRegionNumberRegister.REGION];
    }

    inline auto MPU_RASR(uint8_t region) -> MpuRegionAttributeAndSizeRegister&
    {
        assert(region < m_mpuRegionAttributeAndSizeRegisters.size());
        return m_mpuRegionAttributeAndSizeRegisters[region];
    }

    inline auto MPU_RASR(uint8_t region) const -> const MpuRegionAttributeAndSizeRegister&
    {
        assert(region < m_mpuRegionAttributeAndSizeRegisters.size());
        return m_mpuRegionAttributeAndSizeRegisters[region];
    }

private:
    MpuTypeRegister m_mpuTypeRegister;
    MpuControlRegister m_mpuControlRegister;
    MpuRegionNumberRegister m_mpuRegionNumberRegister;

    std::array<MpuRegionBaseAddressRegister, 8> m_mpuRegionBaseAddressRegisters;
    std::array<MpuRegionAttributeAndSizeRegister, 8> m_mpuRegionAttributeAndSizeRegisters;
};

}  // namespace stm32::rg
