// This is #include #include "opcodes.hpp""opcodes.hpp"an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "system_control_registers.hpp"

namespace stm32
{
SystemControlRegistersSet::SystemControlRegistersSet()
    : m_cpuIdBaseRegister{}
    , m_interruptControlAndStateRegister{}
    , m_vectorTableOffsetRegister{}
    , m_applicationInterruptAndResetControlRegister{}
    , m_systemControlRegister{}
    , m_configurationAndControlRegister{}
    , m_systemHandlerControlAndStateRegister{}
    , m_configurableFaultStatusRegister{}
    , m_memManageStatusRegister{}
    , m_busFaultStatusRegister{}
    , m_usageFaultStatusRegister{}
    , m_hardFaultStatusRegister{}
    , m_auxiliaryFaultStatusRegister{}
    , m_memManageFaultAddressRegister{}
    , m_busFaultAddressRegister{}
    , m_coprocessorAccessControlRegister{}
    , m_auxiliaryControlRegister{}
    , m_interruptControllerTypeRegister{}
    , m_softwareTriggeredInterruptRegister{}
    , m_sysTickControlAndStatusRegister{}
    , m_sysTickReloadValueRegister{}
    , m_sysTickCurrentValueRegister{}
    , m_sysTickCalibrationValueRegister{}
{
    reset();
}

void SystemControlRegistersSet::reset()
{
    // TODO: reset system registers
}

}  // namespace stm32
