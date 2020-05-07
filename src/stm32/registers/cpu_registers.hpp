#pragma once

#include <cstdint>

#include "../utils/general.hpp"

namespace stm32::rg
{
/**
 * The special-purpose program status registers, xPSR
 */
DEFINE_REG(ApplicationProgramStatusRegister, {
    RESERVE(27);  ///< bits[26:0]

    bool Q : 1;  ///< bit[27]
                 ///< Set to 1 if a SSAT or USAT instruction changes the input value for the signed or unsigned range
                 ///< of the result. In a processor that implements the DSP extension, the processor sets this bit
                 ///< to 1 to indicate an overflow on some multiplies. Setting this bit to 1 is called saturation.

    bool V : 1;  ///< overflow, bit[28]
                 ///< Overflow condition code flag. Set to 1 if the instruction results in an overflow condition, for
                 ///< example a signed overflow on an addition.

    bool C : 1;  ///< carry, bit[29]
                 ///< Carry condition code flag. Set to 1 if the instruction results in a carry condition, for example an
                 ///< unsigned overflow on an addition.

    bool Z : 1;  ///< zero, bit[30]
                 ///< Zero condition code flag. Set to 1 if the result of the instruction is zero, and to 0 otherwise. A
                 ///< result of zero often indicates an equal result from a comparison.

    bool N : 1;  ///< negative, bit[31]
                 ///< Negative condition code flag. Set to bit[31] of the result of the instruction. If the result is
                 ///< regarded as a two's complement signed integer, then N == 1 if the result is negative and N == 0 if
                 ///< it is positive or zero.
});

DEFINE_REG(InterruptProgramStatusRegister, {
    uint16_t exceptionNumber : 9;  ///< bits[8:0]
                                   ///< When the processor is executing an exception handler, holds the exception number
                                   ///< of the exception being processed. Otherwise, the IPSR value is zero.

    RESERVE(23);  ///< bits[31:9]
});

DEFINE_REG(ExecutionProgramStatusRegister, {
    RESERVE(10);  ///< bits[9:0]

    uint8_t ITlo : 6;  ///< bits[15:10]
                       ///< high 6 bits of IT

    bool T : 1;  ///< bit[24]
                 ///< T bit, that is set to 1 to indicate that the processor executes Thumb instructions

    uint8_t IThi : 2;  ///< bits[26:25]
                       ///< low two bits of ITd

    RESERVE(5);  ///< bits[31:27]
});

/**
 * The special-purpose mask registers
 */
DEFINE_REG(ExceptionMaskRegister, {
    bool PM : 1;  ///< bit[0]
                  ///< Setting PRIMASK to 1 raises the execution priority to 0.

    RESERVE(31);  ///< bits[31:1]
});

DEFINE_REG(BasePriorityMaskRegister, {
    uint8_t level : 8;  ///< bit[7:0]
                        ///< Changes the priority level required for exception preemption. It has an effect only when
                        ///< it has a lower value than the unmasked priority level of the currently executing software.

    RESERVE(24);  ///< bits[31:8]
});

DEFINE_REG(FaultMaskRegister, {
    bool FM : 1;  ///< bit[0]
                  ///< Setting FM to 1 raises the execution priority to -1, the priority of HardFault. Only
                  ///< privileged software executing at a priority below -1 can set FM to 1. This means
                  ///< HardFault and NMI handlers cannot set FM to 1.  Returning from any exception except NMI
                  ///< clears FM to 0.

    RESERVE(31);  ///< bits[31:1]
});

/**
 * The special-purpose CONTROL 2-bit register
 */
struct __attribute__((__packed__)) ControlRegister {
    bool nPRIV : 1;  ///< bit[0]
                     ///< Defines the execution privilege in Thread mode
                     ///<
                     ///< false - Thread mode has privileged access.
                     ///<
                     ///< true - Thread mode has unprivileged access.

    bool SPSEL : 1;  ///< bit[1]
                     ///< Defines the stack to be used
                     ///<
                     ///< false - Use SP_main as the current stack.
                     ///<
                     ///< true - In Thread mode, use SP_process as the current stack.
                     ///<        In Handler mode, this value is reserved
};

}  // namespace stm32::rg
