// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "cpu_register_set.hpp"

namespace stm32
{
void CpuRegisterSet::reset()
{
    m_interruptProgramStatusRegister.exceptionNumber = 0x0u;
    m_executionProgramStatusRegister.T = true;

    m_exceptionMaskRegister.PM = false;
    m_basePriorityMaskRegister.level = 0x0u;
    m_faultMaskRegister.FM = false;

    m_controlRegister.nPRIV = false;
    m_controlRegister.SPSEL = false;
}


auto CpuRegisterSet::reg(const RegisterType& reg) -> uint32_t&
{
    switch (reg)
    {
        case RegisterType::R0:
        case RegisterType::R1:
        case RegisterType::R2:
        case RegisterType::R3:
        case RegisterType::R4:
        case RegisterType::R5:
        case RegisterType::R6:
        case RegisterType::R7:
        case RegisterType::R8:
        case RegisterType::R9:
        case RegisterType::R10:
        case RegisterType::R11:
        case RegisterType::R12:
        case RegisterType::R13:
            return m_generalPurposeRegisters[static_cast<int>(reg)];

        case RegisterType::SP:
            if (m_controlRegister.SPSEL)
            {
                // TODO: check if current mode is Thread and return UNPREDICTABLE otherwise
                return m_stackPointers[StackPointerType::Process];
            }
            else
            {
                return m_stackPointers[StackPointerType::Main];
            }

        case RegisterType::LR:
            return m_linkRegister;

        case RegisterType::PC:
            return m_programCounter;
    }
}


auto CpuRegisterSet::xPSR() -> uint32_t&
{
    return m_programStatusRegister;
}


auto CpuRegisterSet::APSR() -> ApplicationProgramStatusRegister&
{
    return m_applicationProgramStatusRegister;
}


auto CpuRegisterSet::IPSR() -> InterruptProgramStatusRegister&
{
    return m_interruptProgramStatusRegister;
}


auto CpuRegisterSet::EPSR() -> ExecutionProgramStatusRegister&
{
    return m_executionProgramStatusRegister;
}


auto CpuRegisterSet::PRIMASK() -> ExceptionMaskRegister&
{
    return m_exceptionMaskRegister;
}


auto CpuRegisterSet::BASEPRI() -> BasePriorityMaskRegister&
{
    return m_basePriorityMaskRegister;
}


auto CpuRegisterSet::FAULTMASK() -> FaultMaskRegister&
{
    return m_faultMaskRegister;
}


auto CpuRegisterSet::CONTROL() -> ControlRegister&
{
    return m_controlRegister;
}


}  // namespace stm32
