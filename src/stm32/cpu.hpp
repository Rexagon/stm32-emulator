#pragma once

#include <bitset>

#include "memory.hpp"
#include "registers/cpu_registers_set.hpp"
#include "registers/nvic_registers_set.hpp"
#include "registers/sys_tick_registers_set.hpp"
#include "registers/system_control_registers_set.hpp"
#include "utils/math.hpp"

namespace stm32
{
enum ExceptionType : uint16_t {
    Reset = 1,
    NMI = 2,
    HardFault = 3,
    MemManage = 4,
    BusFault = 5,
    UsageFault = 6,
    SVCall = 11,
    // DebugMonitor = 12,
    PendSV = 14,
    SysTick = 15,
};

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

    auto isInPrivilegedMode() const -> bool;
    auto executionPriority() const -> int32_t;

    void pushStack(ExceptionType exceptionType);
    void exceptionTaken(ExceptionType exceptionType);

    inline void setEventRegister() { m_eventRegister = true; }
    inline void clearEventRegister() { m_eventRegister = false; }
    inline auto wasEventRegistered() -> bool { return m_eventRegister; }

    inline auto R(uint8_t reg) const -> uint32_t { return m_registers.getRegister(reg); }
    inline void setR(uint8_t reg, uint32_t value) { m_registers.setRegister(reg, value); }

    inline auto registers() -> rg::CpuRegistersSet& { return m_registers; }
    inline auto systemRegisters() -> rg::SystemControlRegistersSet& { return m_systemRegisters; }
    inline auto sysTickRegisters() -> rg::SysTickRegistersSet& { return m_sysTickRegisters; }
    inline auto nvicRegisters() -> rg::NvicRegistersSet& { return m_nvicRegisters; }

    inline auto memory() -> Memory& { return m_memory; }

private:
    rg::CpuRegistersSet m_registers;
    rg::SystemControlRegistersSet m_systemRegisters;
    rg::SysTickRegistersSet m_sysTickRegisters;
    rg::NvicRegistersSet m_nvicRegisters;
    Memory m_memory;

    ExecutionMode m_currentMode;
    std::bitset<512> m_exceptionActive;

    bool m_eventRegister = false;
};

template <typename T>
auto Cpu::alignedMemoryRead(uint32_t address) -> T
{
    if (!utils::isAddressAligned<T>(address)) {
        m_systemRegisters.UFSR().UNALIGNED_ = true;
        // TODO: ExceptionTaken(UsageFault)
    }
}

}  // namespace stm32
