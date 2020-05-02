// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "cpu_register_set.hpp"

#include <cassert>

#include "math.hpp"

namespace stm32
{
CpuRegisterSet::CpuRegisterSet()
    : m_exceptionMaskRegister{}
    , m_basePriorityMaskRegister{}
    , m_faultMaskRegister{}
    , m_controlRegister{}
{
}

auto CpuRegisterSet::reg(uint16_t reg) -> uint32_t&
{
    switch (reg) {
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
            return m_generalPurposeRegisters[static_cast<int>(reg)];

        case RegisterType::SP:
            if (m_controlRegister.SPSEL) {
                // TODO: check if current mode is Thread and return UNPREDICTABLE otherwise
                return m_stackPointers[StackPointerType::Process];
            }
            else {
                return m_stackPointers[StackPointerType::Main];
            }

        case RegisterType::LR:
            return m_linkRegister;

        case RegisterType::PC:
            return m_programCounter;

        default:
            UNPREDICTABLE;
    }
}

auto CpuRegisterSet::SP() -> uint32_t&
{
    if (m_controlRegister.SPSEL) {
        // TODO: check if current mode is Thread and return UNPREDICTABLE otherwise
        return m_stackPointers[StackPointerType::Process];
    }
    else {
        return m_stackPointers[StackPointerType::Main];
    }
}

auto CpuRegisterSet::SP_main() -> uint32_t&
{
    return m_stackPointers[StackPointerType::Main];
}

auto CpuRegisterSet::SP_process() -> uint32_t&
{
    return m_stackPointers[StackPointerType::Process];
}

}  // namespace stm32
