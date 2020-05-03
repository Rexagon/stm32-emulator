// This is #include #include "opcodes.hpp""opcodes.hpp"an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "system_control_registers_set.hpp"

#include "../utils.hpp"

namespace stm32::sc
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
    resetRegisterValue(m_cpuIdBaseRegister, 0x412FC231u);
    resetRegisterValue(m_interruptControlAndStateRegister);
    resetRegisterValue(m_vectorTableOffsetRegister);
    resetRegisterValue(m_applicationInterruptAndResetControlRegister);
    resetRegisterValue(m_systemControlRegister);
    resetRegisterValue(m_configurationAndControlRegister);

    resetRegisterValue(m_systemHandlerPriorityRegisters[0]);
    resetRegisterValue(m_systemHandlerPriorityRegisters[1]);
    resetRegisterValue(m_systemHandlerPriorityRegisters[2]);
    resetRegisterValue(m_systemHandlerControlAndStateRegister);

    resetRegisterValue(m_configurationAndControlRegister);
    resetRegisterValue(m_hardFaultStatusRegister);
    resetRegisterValue(m_auxiliaryFaultStatusRegister);

    // MMFAR is unknown on reset
    // BFAR is unknown on reset

    resetRegisterValue(m_coprocessorAccessControlRegister);
    resetRegisterValue(m_auxiliaryControlRegister);

    resetRegisterValue(m_interruptControllerTypeRegister, 0x00000111u);  // TODO: check value on real microcontroller

    resetRegisterValue(m_softwareTriggeredInterruptRegister);
}

}  // namespace stm32::sc
