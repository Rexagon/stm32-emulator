// This is #include #include "opcodes.hpp""opcodes.hpp"an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "system_control_registers_set.hpp"

#include "../utils/general.hpp"

namespace stm32::rg
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
    , m_hardFaultStatusRegister{}
    , m_auxiliaryFaultStatusRegister{}
    , m_memManageFaultAddressRegister{}
    , m_busFaultAddressRegister{}
    , m_coprocessorAccessControlRegister{}
    , m_auxiliaryControlRegister{}
    , m_interruptControllerTypeRegister{}
    , m_softwareTriggeredInterruptRegister{}
{
    reset();
}

void SystemControlRegistersSet::reset()
{
    m_cpuIdBaseRegister.registerData = 0x412FC231u;
    m_interruptControlAndStateRegister.registerData = 0u;
    m_vectorTableOffsetRegister.registerData = 0u;
    m_applicationInterruptAndResetControlRegister.registerData = 0u;
    m_systemControlRegister.registerData = 0u;
    m_configurationAndControlRegister.registerData = 0u;

    m_systemHandlerPriorityRegisters[0].registerData = 0u;
    m_systemHandlerPriorityRegisters[1].registerData = 0u;
    m_systemHandlerPriorityRegisters[2].registerData = 0u;
    m_systemHandlerControlAndStateRegister.registerData = 0u;

    m_configurationAndControlRegister.registerData = 0u;
    m_hardFaultStatusRegister.registerData = 0u;
    m_auxiliaryFaultStatusRegister.registerData = 0u;

    // MMFAR is unknown on reset
    // BFAR is unknown on reset

    m_coprocessorAccessControlRegister.registerData = 0u;
    m_auxiliaryControlRegister.registerData = 0u;

    m_interruptControllerTypeRegister.registerData = 0x00000111u;  // TODO: check value on real microcontroller

    m_softwareTriggeredInterruptRegister.registerData = 0u;
}

}  // namespace stm32::sc
