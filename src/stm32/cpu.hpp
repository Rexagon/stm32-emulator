#pragma once

#include <cstdint>

namespace stm32
{
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
};

struct InterruptProgramStatusRegister
{
    uint16_t exceptionNumber : 9;  // bits[8:0]
                                   // When the processor is executing an exception handler, holds the exception number
                                   // of the exception being processed. Otherwise, the IPSR value is zero.

    uint16_t : 7;   // bits[15:9]
    uint16_t : 16;  // bits[31:16]
                    // Reserved
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
};

struct CpuRegisterSet
{
    uint32_t R[16]{};

    uint32_t &SP = R[13];
    uint32_t &LR = R[14];
    uint32_t &PC = R[15];

    union {
        uint32_t xPSR = 0x1u << ExecutionProgramStatusRegister::FlagTBitNumber;

        ApplicationProgramStatusRegister APSR;
        InterruptProgramStatusRegister IPSR;
        ExecutionProgramStatusRegister EPSR;
    };
};

}  // namespace stm32
