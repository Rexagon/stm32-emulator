// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "mpu_registers_set.hpp"

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

}  // namespace stm32::rg
