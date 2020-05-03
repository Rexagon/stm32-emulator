// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "nvic_registers_set.hpp"

#include <cassert>

#include "../utils.hpp"

namespace stm32::sc
{
NvicRegistersSet::NvicRegistersSet()
    : m_interruptEnableStates{}
    , m_interruptPendingStates{}
    , m_interruptActiveBitRegisters{}
    , m_interruptPriorityRegister{}
{
    reset();
}

void NvicRegistersSet::reset()
{
    for (size_t i = 0; i < 8; ++i) {
        resetRegisterValue(m_interruptEnableStates[i]);
    }
}

auto NvicRegistersSet::ISER(uint8_t n) const -> const InterruptSetEnableRegister&
{
    assert(n < 8);
    return *reinterpret_cast<const InterruptSetEnableRegister*>(m_interruptEnableStates + n);
}

auto NvicRegistersSet::ICER(uint8_t n) const -> const InterruptClearEnableRegister&
{
    assert(n < 8);
    return *reinterpret_cast<const InterruptClearEnableRegister*>(m_interruptEnableStates + n);
}

auto NvicRegistersSet::ISPR(uint8_t n) const -> const InterruptSetPendingRegister&
{
    assert(n < 8);
    return *reinterpret_cast<const InterruptSetPendingRegister*>(m_interruptPendingStates + n);
}

auto NvicRegistersSet::ICPR(uint8_t n) const -> const InterruptClearPendingRegister&
{
    assert(n < 8);
    return *reinterpret_cast<const InterruptClearPendingRegister*>(m_interruptPendingStates + n);
}

auto NvicRegistersSet::IABR(uint8_t n) const -> const InterruptActiveBitRegister&
{
    assert(n < 8);
    return m_interruptActiveBitRegisters[n];
}

auto NvicRegistersSet::IPR(uint8_t n) const -> const InterruptPriorityRegister&
{
    assert(n < 60);
    return m_interruptPriorityRegister[n];
}

}  // namespace stm32::sc
