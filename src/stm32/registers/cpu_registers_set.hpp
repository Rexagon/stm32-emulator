#pragma once

#include <array>

#include "cpu_registers.hpp"

namespace stm32::rg
{
enum RegisterType : uint8_t {
    R0 = 0u,
    R1 = 1u,
    R2 = 2u,
    R3 = 3u,
    R4 = 4u,
    R5 = 5u,
    R6 = 6u,
    R7 = 7u,
    R8 = 8u,
    R9 = 9u,
    R10 = 10u,
    R11 = 11u,
    R12 = 12u,
    SP = 13u,
    LR = 14u,
    PC = 15u,
};

class CpuRegistersSet {
    enum StackPointerType {
        Main = 0,
        Process = 1,
    };

public:
    explicit CpuRegistersSet();

    void reset();

    auto getRegister(uint8_t reg) const -> uint32_t;
    void setRegister(uint8_t reg, uint32_t value);

    auto SP() -> uint32_t&;
    auto SP() const -> const uint32_t&;

    inline auto SP_main() -> uint32_t& { return m_stackPointers[StackPointerType::Main]; }
    inline auto SP_main() const -> const uint32_t& { return m_stackPointers[StackPointerType::Main]; }

    inline auto SP_process() -> uint32_t& { return m_stackPointers[StackPointerType::Process]; }
    inline auto SP_process() const -> const uint32_t& { return m_stackPointers[StackPointerType::Process]; }

    inline auto LR() -> uint32_t& { return m_linkRegister; }
    inline auto LR() const -> const uint32_t& { return m_linkRegister; }

    inline auto PC() -> uint32_t& { return m_programCounter; }
    inline auto PC() const -> const uint32_t& { return m_programCounter; }

    inline auto xPSR() -> uint32_t& { return m_programStatusRegister; }
    inline auto xPSR() const -> const uint32_t& { return m_programStatusRegister; }

    inline auto APSR() -> ApplicationProgramStatusRegister& { return m_applicationProgramStatusRegister; }
    inline auto APSR() const -> const ApplicationProgramStatusRegister& { return m_applicationProgramStatusRegister; }

    inline auto IPSR() -> InterruptProgramStatusRegister& { return m_interruptProgramStatusRegister; }
    inline auto IPSR() const -> const InterruptProgramStatusRegister& { return m_interruptProgramStatusRegister; }

    inline auto EPSR() -> ExecutionProgramStatusRegister& { return m_executionProgramStatusRegister; }
    inline auto EPSR() const -> const ExecutionProgramStatusRegister& { return m_executionProgramStatusRegister; }

    inline auto PRIMASK() -> ExceptionMaskRegister& { return m_exceptionMaskRegister; }
    inline auto PRIMASK() const -> const ExceptionMaskRegister& { return m_exceptionMaskRegister; }

    inline auto BASEPRI() -> BasePriorityMaskRegister& { return m_basePriorityMaskRegister; }
    inline auto BASEPRI() const -> const BasePriorityMaskRegister& { return m_basePriorityMaskRegister; }

    inline auto FAULTMASK() -> FaultMaskRegister& { return m_faultMaskRegister; }
    inline auto FAULTMASK() const -> const FaultMaskRegister& { return m_faultMaskRegister; }

    inline auto CONTROL() -> ControlRegister& { return m_controlRegister; }
    inline auto CONTROL() const -> const ControlRegister& { return m_controlRegister; }

    auto ITSTATE() const -> uint8_t;
    void setITSTATE(uint8_t value);

private:
    std::array<uint32_t, 12> m_generalPurposeRegisters{};
    std::array<uint32_t, 2> m_stackPointers{};
    uint32_t m_linkRegister{};
    uint32_t m_programCounter{};

    union {
        uint32_t m_programStatusRegister;

        ApplicationProgramStatusRegister m_applicationProgramStatusRegister;
        InterruptProgramStatusRegister m_interruptProgramStatusRegister;
        ExecutionProgramStatusRegister m_executionProgramStatusRegister;
    };

    ExceptionMaskRegister m_exceptionMaskRegister;
    BasePriorityMaskRegister m_basePriorityMaskRegister;
    FaultMaskRegister m_faultMaskRegister;

    ControlRegister m_controlRegister;
};

}  // namespace stm32::rg
