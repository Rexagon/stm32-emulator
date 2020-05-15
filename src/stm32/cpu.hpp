#pragma once

#include <bitset>

#include "memory.hpp"
#include "mpu.hpp"
#include "registers/cpu_registers_set.hpp"
#include "registers/nvic_registers_set.hpp"
#include "registers/sys_tick_registers_set.hpp"
#include "registers/system_control_registers_set.hpp"
#include "utils/math.hpp"

namespace stm32
{
/**
 * The M-profile execution modes
 */
enum class ExecutionMode {
    Thread,
    Handler,
};

class Cpu {
public:
    explicit Cpu(const Memory::Config& memoryConfig);

    void reset();
    void step();

    void branchWritePC(uint32_t address, bool skipIncrementingPC = true);
    void bxWritePC(uint32_t address, bool skipIncrementingPC = true);
    void blxWritePC(uint32_t address, bool skipIncrementingPC = true);
    inline void loadWritePC(uint32_t address) { bxWritePC(address); }
    inline void aluWritePC(uint32_t address) { branchWritePC(address); }

    inline auto currentMode() -> ExecutionMode& { return m_currentMode; }

    auto currentCondition() const -> uint8_t;
    auto conditionPassed() const -> bool;
    auto isInItBlock() const -> bool;
    auto isLastInItBlock() const -> bool;
    void advanceCondition();

    auto isInPrivilegedMode() const -> bool;
    auto executionPriority() const -> int32_t;

    void exceptionEntry(uint16_t exceptionType);
    void pushStack(uint16_t exceptionType);
    void exceptionTaken(uint16_t exceptionType);
    auto returnAddress(uint16_t exceptionType) -> uint32_t;

    auto currentInstructionAddress() const { return m_currentInstructionAddress; }
    auto nextInstructionAddress() const -> uint32_t;

    void dataMemoryBarrier(uint8_t option);
    void dataSynchronizationBarrier(uint8_t option);
    void instructionSynchronizationBarrier(uint8_t option);
    void preloadData(uint32_t address);
    void preloadInstruction(uint32_t address);

    inline void setEventRegister() { m_wasEventRegistered = true; }
    inline void clearEventRegister() { m_wasEventRegistered = false; }
    inline auto wasEventRegistered() -> bool { return m_wasEventRegistered; }

    inline auto R(uint8_t reg) const -> uint32_t { return m_registers.getRegister(reg); }
    inline void setR(uint8_t reg, uint32_t value) { m_registers.setRegister(reg, value); }
    inline auto registers() -> rg::CpuRegistersSet& { return m_registers; }

    inline auto systemRegisters() -> rg::SystemControlRegistersSet& { return m_systemRegisters; }

    inline auto sysTickRegisters() -> rg::SysTickRegistersSet& { return m_sysTickRegisters; }

    inline auto nvicRegisters() -> rg::NvicRegistersSet& { return m_nvicRegisters; }

    inline auto mpu() -> Mpu& { return m_mpu; }

    inline auto memory() -> Memory& { return m_memory; }

private:
    rg::CpuRegistersSet m_registers;
    rg::SystemControlRegistersSet m_systemRegisters;
    rg::SysTickRegistersSet m_sysTickRegisters;
    rg::NvicRegistersSet m_nvicRegisters;
    Memory m_memory;

    Mpu m_mpu;

    ExecutionMode m_currentMode;
    std::bitset<512> m_exceptionActive;
    bool m_wasEventRegistered = false;

    bool m_skipIncrementingPC = false;
    uint32_t m_currentInstructionAddress = 0u;
    uint32_t m_nextInstructionAddress = 0u;
};

}  // namespace stm32
