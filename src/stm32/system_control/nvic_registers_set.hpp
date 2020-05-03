#pragma once

#include "nvic_registers.hpp"

namespace stm32::sc
{
class NvicRegistersSet {
public:
    explicit NvicRegistersSet();

    void reset();

    auto ISER(uint8_t n) const -> const InterruptSetEnableRegister&;
    auto ICER(uint8_t n) const -> const InterruptClearEnableRegister&;

    auto ISPR(uint8_t n) const -> const InterruptSetPendingRegister&;
    auto ICPR(uint8_t n) const -> const InterruptClearPendingRegister&;

    auto IABR(uint8_t n) const -> const InterruptActiveBitRegister&;

    auto IPR(uint8_t n) const -> const InterruptPriorityRegister&;

private:
    uint32_t m_interruptEnableStates[8];
    uint32_t m_interruptPendingStates[8];

    InterruptActiveBitRegister m_interruptActiveBitRegisters[8];
    InterruptPriorityRegister m_interruptPriorityRegister[60];
};

}  // namespace stm32::sc
