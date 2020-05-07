#pragma once

#include "system_control_registers.hpp"

namespace stm32::rg
{
class SystemControlRegistersSet {
public:
    explicit SystemControlRegistersSet();

    void reset();

    inline auto CPUID() const -> const CpuIdBaseRegister& { return m_cpuIdBaseRegister; }
    inline auto ICSR() const -> const InterruptControlAndStateRegister& { return m_interruptControlAndStateRegister; }
    inline auto VTOR() const -> const VectorTableOffsetRegister& { return m_vectorTableOffsetRegister; }
    inline auto AIRCR() const -> const ApplicationInterruptAndResetControlRegister&
    {
        return m_applicationInterruptAndResetControlRegister;
    }
    inline auto SCR() const -> const SystemControlRegister& { return m_systemControlRegister; }
    inline auto CCR() const -> const ConfigurationAndControlRegister& { return m_configurationAndControlRegister; }

    inline auto SHPR1() const -> const SystemHandlerPriorityRegister& { return m_systemHandlerPriorityRegisters[0]; }
    inline auto SHPR2() const -> const SystemHandlerPriorityRegister& { return m_systemHandlerPriorityRegisters[1]; }
    inline auto SHPR3() const -> const SystemHandlerPriorityRegister& { return m_systemHandlerPriorityRegisters[2]; }
    inline auto SHCSR() const -> const SystemHandlerControlAndStateRegister& { return m_systemHandlerControlAndStateRegister; }

    inline auto CFSR() -> ConfigurableFaultStatusRegister& { return m_configurableFaultStatusRegister; }
    inline auto CFSR() const -> const ConfigurableFaultStatusRegister& { return m_configurableFaultStatusRegister; }

    inline auto HFSR() const -> const HardFaultStatusRegister& { return m_hardFaultStatusRegister; }
    inline auto AFSR() const -> const AuxiliaryFaultStatusRegister& { return m_auxiliaryFaultStatusRegister; }

    inline auto MMFAR() const -> const MemManageFaultAddressRegister& { return m_memManageFaultAddressRegister; }
    inline auto BFAR() const -> const BusFaultAddressRegister& { return m_busFaultAddressRegister; }

    inline auto CPACR() const -> const CoprocessorAccessControlRegister& { return m_coprocessorAccessControlRegister; }

    inline auto ICTR() const -> const InterruptControllerTypeRegister& { return m_interruptControllerTypeRegister; }
    inline auto ACTLR() const -> const AuxiliaryControlRegister& { return m_auxiliaryControlRegister; }

    inline auto STIR() const -> const SoftwareTriggeredInterruptRegister& { return m_softwareTriggeredInterruptRegister; }

private:
    CpuIdBaseRegister m_cpuIdBaseRegister;
    InterruptControlAndStateRegister m_interruptControlAndStateRegister;
    VectorTableOffsetRegister m_vectorTableOffsetRegister;
    ApplicationInterruptAndResetControlRegister m_applicationInterruptAndResetControlRegister;
    SystemControlRegister m_systemControlRegister;
    ConfigurationAndControlRegister m_configurationAndControlRegister;

    SystemHandlerPriorityRegister m_systemHandlerPriorityRegisters[3];
    SystemHandlerControlAndStateRegister m_systemHandlerControlAndStateRegister;

    ConfigurableFaultStatusRegister m_configurableFaultStatusRegister;
    HardFaultStatusRegister m_hardFaultStatusRegister;
    AuxiliaryFaultStatusRegister m_auxiliaryFaultStatusRegister;

    MemManageFaultAddressRegister m_memManageFaultAddressRegister;
    BusFaultAddressRegister m_busFaultAddressRegister;

    CoprocessorAccessControlRegister m_coprocessorAccessControlRegister;
    AuxiliaryControlRegister m_auxiliaryControlRegister;

    InterruptControllerTypeRegister m_interruptControllerTypeRegister;

    SoftwareTriggeredInterruptRegister m_softwareTriggeredInterruptRegister;
};

}  // namespace stm32::sc
