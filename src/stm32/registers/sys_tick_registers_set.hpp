#pragma once

#include "sys_tick_registers.hpp"

namespace stm32::rg
{
class SysTickRegistersSet {
public:
    explicit SysTickRegistersSet();

    void reset();

    inline auto SYST_CSR() const -> const SysTickControlAndStatusRegister& { return m_sysTickControlAndStatusRegister; }
    inline auto SYST_RVR() const -> const SysTickReloadValueRegister& { return m_sysTickReloadValueRegister; }
    inline auto SYST_CVR() const -> const SysTickCurrentValueRegister& { return m_sysTickCurrentValueRegister; }
    inline auto SYST_CALIB() const -> const SysTickCalibrationValueRegister& { return m_sysTickCalibrationValueRegister; }

private:
    SysTickControlAndStatusRegister m_sysTickControlAndStatusRegister;
    SysTickReloadValueRegister m_sysTickReloadValueRegister;
    SysTickCurrentValueRegister m_sysTickCurrentValueRegister;
    SysTickCalibrationValueRegister m_sysTickCalibrationValueRegister;
};

}  // namespace stm32::sc
