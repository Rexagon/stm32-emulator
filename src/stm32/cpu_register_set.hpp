#pragma once

#include <cstdint>

namespace stm32
{
// The special-purpose program status registers, xPSR
//

struct __attribute__((__packed__)) ApplicationProgramStatusRegister {
    uint32_t : 27;  // bits[26:0]
                    // Reserved

    static constexpr uint8_t FlagQBitNumber = 27;
    bool Q : 1;  // bit[27]
                 // Set to 1 if a SSAT or USAT instruction changes the input value for the signed or unsigned range
                 // of the result. In a processor that implements the DSP extension, the processor sets this bit
                 // to 1 to indicate an overflow on some multiplies. Setting this bit to 1 is called saturation.

    static constexpr uint8_t FlagVBitNumber = 28;
    bool V : 1;  // overflow, bit[28]
                 // Overflow condition code flag. Set to 1 if the instruction results in an overflow condition, for
                 // example a signed overflow on an addition.

    static constexpr uint8_t FlagCBitNumber = 29;
    bool C : 1;  // carry, bit[29]
                 // Carry condition code flag. Set to 1 if the instruction results in a carry condition, for example an
                 // unsigned overflow on an addition.

    static constexpr uint8_t FlagZBitNumber = 30;
    bool Z : 1;  // zero, bit[30]
                 // Zero condition code flag. Set to 1 if the result of the instruction is zero, and to 0 otherwise. A
                 // result of zero often indicates an equal result from a comparison.

    static constexpr uint8_t FlagNBitNumber = 31;
    bool N : 1;  // negative, bit[31]
                 // Negative condition code flag. Set to bit[31] of the result of the instruction. If the result is
                 // regarded as a two's complement signed integer, then N == 1 if the result is negative and N == 0 if
                 // it is positive or zero.
};

struct __attribute__((__packed__)) InterruptProgramStatusRegister {
    uint16_t exceptionNumber : 9;  // bits[8:0]
                                   // When the processor is executing an exception handler, holds the exception number
                                   // of the exception being processed. Otherwise, the IPSR value is zero.

    uint32_t : 23;  // bits[31:9]
                    // Reserved
};

struct __attribute__((__packed__)) ExecutionProgramStatusRegister {
    uint16_t : 10;  // bits[9:0]
                    // Reserved

    uint8_t ITlo : 6;  // bits[15:10]
                       // low 6 bits of IT

    static constexpr uint8_t FlagTBitNumber = 24;
    bool T : 1;  // bit[24]
                 // T bit, that is set to 1 to indicate that the processor executes Thumb instructions

    uint8_t IThi : 2;  // bits[26:25]
                       // high two bits of IT

    uint8_t : 5;  // bits[31:27]
                  // Reserved
};

// The special-purpose mask registers
//

struct __attribute__((__packed__)) ExceptionMaskRegister {
    bool PM : 1;  // bit[0]
                  // Setting PRIMASK to 1 raises the execution priority to 0.

    uint32_t : 31;  // bits[31:1]
                    // Reserved
};

struct __attribute__((__packed__)) BasePriorityMaskRegister {
    uint8_t level : 8;  // bit[7:0]
                        // Changes the priority level required for exception preemption. It has an effect only when
                        // it has a lower value than the unmasked priority level of the currently executing software.

    uint32_t : 24;  // bits[31:8]
                    // Reserved
};

struct __attribute__((__packed__)) FaultMaskRegister {
    bool FM : 1;  // bit[0]
                  // Setting FM to 1 raises the execution priority to -1, the priority of HardFault. Only
                  // privileged software executing at a priority below -1 can set FM to 1. This means
                  // HardFault and NMI handlers cannot set FM to 1.  Returning from any exception except NMI
                  // clears FM to 0.

    uint32_t : 31;  // bits[31:1]
                    // Reserved
};

// The special-purpose CONTROL 2-bit register
//
struct __attribute__((__packed__)) ControlRegister {
    bool nPRIV : 1;  // bit[0]
                     // Defines the execution privilege in Thread mode
                     // false - Thread mode has privileged access.
                     // true - Thread mode has unprivileged access.

    bool SPSEL : 1;  // bit[1]
                     // Defines the stack to be used
                     // false - Use SP_main as the current stack.
                     // true - In Thread mode, use SP_process as the current stack.
                     //        In Handler mode, this value is reserved
};

// The M-profile execution modes
enum class ExecutionMode {
    Thread,
    Handler,
};


// Register set
//

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

class CpuRegisterSet {
    enum StackPointerType {
        Main = 0,
        Process = 1,

        Count
    };

public:
    explicit CpuRegisterSet();

    auto reg(uint16_t reg) -> uint32_t&;

    auto SP_main() -> uint32_t&;
    auto SP_process() -> uint32_t&;

    inline auto xPSR() -> uint32_t& { return m_programStatusRegister; }
    inline auto APSR() -> ApplicationProgramStatusRegister& { return m_applicationProgramStatusRegister; }
    inline auto IPSR() -> InterruptProgramStatusRegister& { return m_interruptProgramStatusRegister; }
    inline auto EPSR() -> ExecutionProgramStatusRegister& { return m_executionProgramStatusRegister; }

    inline auto PRIMASK() -> ExceptionMaskRegister& { return m_exceptionMaskRegister; }
    inline auto BASEPRI() -> BasePriorityMaskRegister& { return m_basePriorityMaskRegister; }
    inline auto FAULTMASK() -> FaultMaskRegister& { return m_faultMaskRegister; }

    inline auto CONTROL() -> ControlRegister& { return m_controlRegister; }

    inline auto ITSTATE() -> uint8_t& { return m_ifThenState; }

    inline auto currentMode() -> ExecutionMode& { return m_currentMode; }

    void branchWritePC(uint32_t address);
    void bxWritePC(uint32_t address);
    void blxWritePC(uint32_t address);
    inline void loadWritePC(uint32_t address) { bxWritePC(address); }
    inline void aluWritePC(uint32_t address) { branchWritePC(address); }

    auto currentCondition() const -> uint8_t;
    auto conditionPassed() const -> bool;
    auto isInItBlock() const -> bool;
    auto isLastInItBlock() const -> bool;
    void advanceCondition();

private:
    uint32_t m_generalPurposeRegisters[12]{};
    uint32_t m_stackPointers[StackPointerType::Count];
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

    ExecutionMode m_currentMode;

    uint8_t m_ifThenState;
};

}  // namespace stm32
