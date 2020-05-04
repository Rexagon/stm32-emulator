// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "cpu_registers_set.hpp"

#include <cassert>

#include "stm32/utils/math.hpp"

namespace stm32::rg
{
CpuRegistersSet::CpuRegistersSet()
    : m_exceptionMaskRegister{}
    , m_basePriorityMaskRegister{}
    , m_faultMaskRegister{}
    , m_controlRegister{}
{
}

void CpuRegistersSet::reset()
{
    m_exceptionMaskRegister.PM = false;
    m_faultMaskRegister.FM = false;
    m_basePriorityMaskRegister.level = 0u;

    m_controlRegister.nPRIV = false;
    m_controlRegister.SPSEL = false;
}

auto CpuRegistersSet::reg(uint16_t reg) -> uint32_t&
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

auto CpuRegistersSet::SP() -> uint32_t&
{
    if (m_controlRegister.SPSEL) {
        // TODO: check if current mode is Thread and return UNPREDICTABLE otherwise
        return m_stackPointers[StackPointerType::Process];
    }
    else {
        return m_stackPointers[StackPointerType::Main];
    }
}

auto CpuRegistersSet::SP_main() -> uint32_t&
{
    return m_stackPointers[StackPointerType::Main];
}

auto CpuRegistersSet::SP_process() -> uint32_t&
{
    return m_stackPointers[StackPointerType::Process];
}

}  // namespace stm32
