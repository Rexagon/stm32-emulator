#pragma once

#include <bitset>

#include "cpu_register_set.hpp"
#include "math.hpp"
#include "memory.hpp"
#include "system_control_registers.hpp"

namespace stm32
{
class Cpu {
public:
    explicit Cpu(const Memory::Config& memoryConfig);

    void reset();
    void step();

    void branchWritePC(uint32_t address);
    void bxWritePC(uint32_t address);
    void blxWritePC(uint32_t address);
    inline void loadWritePC(uint32_t address) { bxWritePC(address); }
    inline void aluWritePC(uint32_t address) { branchWritePC(address); }

    template <typename T>
    auto basicMemoryRead(AddressDescriptor desc) -> T;
    template <typename T>
    auto alignedMemoryRead(uint32_t address) -> T;

    inline auto currentMode() -> ExecutionMode& { return m_currentMode; }

    auto currentCondition() const -> uint8_t;
    auto conditionPassed() const -> bool;
    auto isInItBlock() const -> bool;
    auto isLastInItBlock() const -> bool;
    void advanceCondition();

    auto isInPrivelegedMode() const -> bool;
    auto executionPriority() const -> int32_t;

    inline void setEventRegister() { m_eventRegister = true; }
    inline void clearEventRegister() { m_eventRegister = false; }
    inline auto wasEventRegistered() -> bool { return m_eventRegister; }

    inline auto registers() -> CpuRegisterSet& { return m_registers; }
    inline auto systemRegisters() -> SystemControlRegistersSet& { return m_systemRegisters; }
    inline auto memory() -> Memory& { return m_memory; }

private:
    CpuRegisterSet m_registers;
    SystemControlRegistersSet m_systemRegisters;
    Memory m_memory;

    ExecutionMode m_currentMode;
    std::bitset<512> m_exceptionActive;

    bool m_eventRegister = false;

    uint8_t m_ifThenState;
};

template <typename T>
auto Cpu::alignedMemoryRead(uint32_t address) -> T
{
    if (!math::isAddressAligned<T>(address)) {
        m_systemRegisters.UFSR().UNALIGNED = true;
        // TODO: ExceptionTaken(UsageFault)
    }
}

}  // namespace stm32
