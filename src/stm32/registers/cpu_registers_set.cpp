// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "cpu_registers_set.hpp"

#include <cassert>

#include "../utils/math.hpp"

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

auto CpuRegistersSet::getRegister(uint8_t reg) const -> uint32_t
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
            return m_generalPurposeRegisters[reg];

        case RegisterType::SP:
            return SP() & utils::ZEROS<2, uint32_t>;

        case RegisterType::LR:
            return m_linkRegister;

        case RegisterType::PC:
            return m_programCounter + 4u;

        default:
            UNPREDICTABLE;
    }
}

void CpuRegistersSet::setRegister(uint8_t reg, uint32_t value)
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
            m_generalPurposeRegisters[reg] = value;
            break;

        case RegisterType::SP:
            SP() = value & utils::ZEROS<2, uint32_t>;
            break;

        case RegisterType::LR:
            m_linkRegister = value;
            break;

        default:
            UNPREDICTABLE;
    }
}

auto CpuRegistersSet::SP() -> uint32_t&
{
    if (m_controlRegister.SPSEL) {
        return m_stackPointers[StackPointerType::Process];
    }
    else {
        return m_stackPointers[StackPointerType::Main];
    }
}

auto CpuRegistersSet::SP() const -> const uint32_t&
{
    if (m_controlRegister.SPSEL) {
        return m_stackPointers[StackPointerType::Process];
    }
    else {
        return m_stackPointers[StackPointerType::Main];
    }
}

auto CpuRegistersSet::ITSTATE() const -> uint8_t
{
    using namespace utils;

    return combine<uint8_t>(Part<0, 2>{m_executionProgramStatusRegister.IThi}, Part<2, 6>{m_executionProgramStatusRegister.ITlo});
}

void CpuRegistersSet::setITSTATE(uint8_t value)
{
    using namespace utils;

    m_executionProgramStatusRegister.ITlo = utils::getPart<2, 6>(value) & ONES<6, uint8_t>;
    m_executionProgramStatusRegister.IThi = getPart<0, 2>(value) & ONES<2, uint8_t>;
}

}  // namespace stm32::rg
