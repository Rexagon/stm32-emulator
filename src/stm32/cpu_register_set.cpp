// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "cpu_register_set.hpp"

namespace stm32
{
void InterruptProgramStatusRegister::reset()
{
    exceptionNumber = 0x0u;
}


void ExecutionProgramStatusRegister::reset()
{
    T = true;
}


void ExceptionMaskRegister::reset()
{
    PM = false;
}


void BasePriorityMaskRegister::reset()
{
    level = 0x0u;
}


void FaultMaskRegister::reset()
{
    FM = false;
}


void ControlRegister::reset()
{
    nPRIV = false;
    SPSEL = false;
}


void CpuRegisterSet::reset()
{
    m_interruptProgramStatusRegister.reset();
    m_executionProgramStatusRegister.reset();

    m_exceptionMaskRegister.reset();
    m_basePriorityMaskRegister.reset();
    m_faultMaskRegister.reset();

    m_controlRegister.reset();
}


uint32_t &CpuRegisterSet::reg(const RegisterType &reg)
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


uint32_t &CpuRegisterSet::xPSR()
{
    return m_programStatusRegister;
}


ApplicationProgramStatusRegister &CpuRegisterSet::APSR()
{
    return m_applicationProgramStatusRegister;
}


InterruptProgramStatusRegister &CpuRegisterSet::IPSR()
{
    return m_interruptProgramStatusRegister;
}


ExecutionProgramStatusRegister &CpuRegisterSet::EPSR()
{
    return m_executionProgramStatusRegister;
}


ExceptionMaskRegister &CpuRegisterSet::PRIMASK()
{
    return m_exceptionMaskRegister;
}


BasePriorityMaskRegister &CpuRegisterSet::BASEPRI()
{
    return m_basePriorityMaskRegister;
}


FaultMaskRegister &CpuRegisterSet::FAULTMASK()
{
    return m_faultMaskRegister;
}


ControlRegister &CpuRegisterSet::CONTROL()
{
    return m_controlRegister;
}


}  // namespace stm32
