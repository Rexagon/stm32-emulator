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
    , m_currentMode{}
    , m_ifThenState{}
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

auto CpuRegisterSet::SP_main() -> uint32_t&
{
    return m_stackPointers[StackPointerType::Main];
}

auto CpuRegisterSet::SP_process() -> uint32_t&
{
    return m_stackPointers[StackPointerType::Process];
}

void CpuRegisterSet::branchWritePC(uint32_t address)
{
    m_programCounter = address & ~uint32_t{0b1u};
}

void CpuRegisterSet::bxWritePC(uint32_t address)
{
    if (m_currentMode == ExecutionMode::Handler && math::getPart<28, 4>(address) == 0b1111u) {
        // TODO: ExceptionReturn(address<27:0>); // see B1-597
    }
    else {
        m_executionProgramStatusRegister.T = address & 0b1u;
        m_programCounter = address & ~uint32_t{0b1u};
    }
}

void CpuRegisterSet::blxWritePC(uint32_t address)
{
    m_executionProgramStatusRegister.T = address & 0b1u;
    // TODO: if EPSR.T == 0, a UsageFault(‘Invalid State’) is taken on the next instruction
    m_programCounter = address & ~uint32_t{0b1u};
}

auto CpuRegisterSet::currentCondition() const -> uint8_t
{
    if (m_ifThenState & 0x0fu) {
        return static_cast<uint8_t>(m_ifThenState >> 4u);
    }
    else if (m_ifThenState == 0x00u) {
        return 0b1110u;
    }
    UNPREDICTABLE;
}

auto CpuRegisterSet::conditionPassed() const -> bool
{
    const auto condition = currentCondition() & 0x0fu;
    bool result;
    switch (condition >> 1u) {
        case 0b000u:
            result = m_applicationProgramStatusRegister.Z;
            break;
        case 0b001u:
            result = m_applicationProgramStatusRegister.C;
            break;
        case 0b010u:
            result = m_applicationProgramStatusRegister.N;
            break;
        case 0b011u:
            result = m_applicationProgramStatusRegister.V;
            break;
        case 0b100u:
            result = m_applicationProgramStatusRegister.C && !m_applicationProgramStatusRegister.Z;
            break;
        case 0b101u:
            result = m_applicationProgramStatusRegister.N == m_applicationProgramStatusRegister.V;
            break;
        case 0b110u:
            result = m_applicationProgramStatusRegister.N == m_applicationProgramStatusRegister.V && !m_applicationProgramStatusRegister.Z;
            break;
        case 0b111u:
            result = true;
            break;
        default:
            assert("UNPREDICTABLE");
            result = false;
    }

    if ((condition & 0b1u) && (condition != 0x0fu)) {
        result = !result;
    }

    return result;
}

auto CpuRegisterSet::isInItBlock() const -> bool
{
    return m_ifThenState & 0b1111u;
}

auto CpuRegisterSet::isLastInItBlock() const -> bool
{
    return (m_ifThenState & 0b1111u) == 0b1000u;
}

void CpuRegisterSet::advanceCondition()
{
    if (m_ifThenState & 0b111u) {
        m_ifThenState |= (static_cast<uint8_t>(m_ifThenState << 1u) | 0b1u) & 0b11111u;
    }
    else {
        m_ifThenState = 0x00u;
    }
}

}  // namespace stm32
