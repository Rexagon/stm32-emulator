#pragma once

#include <cstdint>

namespace stm32
{
// The special-purpose program status registers, xPSR
//

struct ApplicationProgramStatusRegister
{
    uint8_t : 8;  // bits[7:0]
    uint8_t : 8;  // bits[15:8]
    uint8_t : 8;  // bits[23:16]
    uint8_t : 3;  // bits[26:24]
                  // Reserved
                  // (multiple uint8_t used to prevent strange uint32_t alignment)

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

    // register shouldn't have reset function
};

struct InterruptProgramStatusRegister
{
    uint16_t exceptionNumber : 9;  // bits[8:0]
                                   // When the processor is executing an exception handler, holds the exception number
                                   // of the exception being processed. Otherwise, the IPSR value is zero.

    uint16_t : 7;   // bits[15:9]
    uint16_t : 16;  // bits[31:16]
                    // Reserved

    void reset();
};

struct ExecutionProgramStatusRegister
{
    uint8_t : 8;  // bits[7:0]
    uint8_t : 8;  // bits[15:8]
    uint8_t : 8;  // bits[23:16]
                  // Reserved

    static constexpr uint8_t FlagTBitNumber = 24;
    bool T : 1;  // bit[24]
                 // T bit, that is set to 1 to indicate that the processor executes Thumb instructions

    void reset();
};

// The special-purpose mask registers
//

struct ExceptionMaskRegister
{
    bool PM : 1;  // bit[0]
                  // Setting PRIMASK to 1 raises the execution priority to 0.

    uint8_t : 7;  // bits[7:1]
    uint8_t : 8;  // bits[15:8]
    uint8_t : 8;  // bits[23:16]
    uint8_t : 8;  // bits[31:24]
                  // Reserved

    void reset();
};

struct BasePriorityMaskRegister
{
    uint8_t level : 8;  // bit[7:0]
                        // Changes the priority level required for exception preemption. It has an effect only when
                        // it has a lower value than the unmasked priority level of the currently executing software.

    uint8_t : 8;  // bits[15:8]
    uint8_t : 8;  // bits[23:16]
    uint8_t : 8;  // bits[31:24]
                  // Reserved

    void reset();
};

struct FaultMaskRegister
{
    uint8_t FM : 1;  // bit[0]
                     // Setting FM to 1 raises the execution priority to -1, the priority of HardFault. Only
                     // privileged software executing at a priority below -1 can set FM to 1. This means
                     // HardFault and NMI handlers cannot set FM to 1.  Returning from any exception except NMI
                     // clears FM to 0.

    uint8_t : 7;  // bits[7:1]
    uint8_t : 8;  // bits[15:8]
    uint8_t : 8;  // bits[23:16]
    uint8_t : 8;  // bits[31:24]
                  // Reserved

    void reset();
};

// The special-purpose CONTROL register
//

struct ControlRegister
{
    bool nPRIV : 1;  // bit[0]
                     // Defines the execution privilege in Thread mode
                     // false - Thread mode has privileged access.
                     // true - Thread mode has unprivileged access.

    bool SPSEL : 1;  // bit[1]
                     // Defines the stack to be used
                     // false - Use SP_main as the current stack.
                     // true - In Thread mode, use SP_process as the current stack.
                     //        In Handler mode, this value is reserved

    void reset();
};

// Register set
//

enum class RegisterType
{
    R0 = 0,
    R1 = 1,
    R2 = 2,
    R3 = 3,
    R4 = 4,
    R5 = 5,
    R6 = 6,
    R7 = 7,
    R8 = 8,
    R9 = 9,
    R10 = 10,
    R11 = 11,
    R12 = 12,
    R13 = 13,
    SP,
    LR,
    PC,
};

class CpuRegisterSet
{
    enum StackPointerType
    {
        Main = 0,
        Process = 1,

        Count
    };

public:
    void reset();

    inline uint32_t &reg(const RegisterType &reg);

    inline uint32_t &xPSR();
    inline ApplicationProgramStatusRegister &APSR();
    inline InterruptProgramStatusRegister &IPSR();
    inline ExecutionProgramStatusRegister &EPSR();

    inline ExceptionMaskRegister &PRIMASK();
    inline BasePriorityMaskRegister &BASEPRI();
    inline FaultMaskRegister &FAULTMASK();

    inline ControlRegister &CONTROL();

private:
    uint32_t m_generalPurposeRegisters[13]{};
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
};

}  // namespace stm32
