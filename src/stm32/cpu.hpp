#pragma once

#include "cpu_register_set.hpp"
#include "memory.hpp"
#include "system_control_registers.hpp"

namespace stm32
{
class Cpu {
public:
    explicit Cpu(const Memory::Config& memoryConfig);

    void reset();
    void step();

    inline auto ITSTATE() -> uint8_t& { return m_ifThenState; }

    void branchWritePC(uint32_t address);
    void bxWritePC(uint32_t address);
    void blxWritePC(uint32_t address);
    inline void loadWritePC(uint32_t address) { bxWritePC(address); }
    inline void aluWritePC(uint32_t address) { branchWritePC(address); }

    inline auto currentMode() -> ExecutionMode& { return m_currentMode; }

    auto currentCondition() const -> uint8_t;
    auto conditionPassed() const -> bool;
    auto isInItBlock() const -> bool;
    auto isLastInItBlock() const -> bool;
    void advanceCondition();

    auto isInPrivelegedMode() const -> bool;
    auto executionPriority() const -> int32_t;

    inline auto registers() -> CpuRegisterSet& { return m_registers; }
    inline auto systemRegisters() -> SystemControlRegistersSet& { return m_systemRegisters; }
    inline auto memory() -> Memory& { return m_memory; }

private:
    CpuRegisterSet m_registers;
    SystemControlRegistersSet m_systemRegisters;
    Memory m_memory;

    ExecutionMode m_currentMode;

    uint8_t m_ifThenState;
};
}  // namespace stm32
