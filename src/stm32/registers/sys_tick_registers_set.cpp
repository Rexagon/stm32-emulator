// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "sys_tick_registers_set.hpp"

#include "stm32/utils/general.hpp"

namespace stm32::rg
{
SysTickRegistersSet::SysTickRegistersSet()
    : m_sysTickControlAndStatusRegister{}
    , m_sysTickReloadValueRegister{}
    , m_sysTickCurrentValueRegister{}
    , m_sysTickCalibrationValueRegister{}
{
}

void SysTickRegistersSet::reset()
{
    resetRegisterValue(m_sysTickControlAndStatusRegister);
    // STRVR is unknown on reset
    // STRVR is unknown on reset
    resetRegisterValue(m_sysTickCalibrationValueRegister);  // TODO: check value on real microcontroller
}

}  // namespace stm32::sc
