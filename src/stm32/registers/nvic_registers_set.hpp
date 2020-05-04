#pragma once

#include <array>

#include "nvic_registers.hpp"

namespace stm32::rg
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
    std::array<uint32_t, 8> m_interruptEnableStates;
    std::array<uint32_t, 8> m_interruptPendingStates;

    std::array<InterruptActiveBitRegister, 8> m_interruptActiveBitRegisters;
    std::array<InterruptPriorityRegister, 60> m_interruptPriorityRegisters;
};

}  // namespace stm32::sc
