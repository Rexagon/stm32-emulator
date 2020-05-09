// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "cpu.hpp"
#include "opcodes.hpp"

namespace
{
inline bool is32bitInstruction(uint16_t opCodeHw1)
{
    const auto value = opCodeHw1 >> 11u;
    return value == 0b11101u || value == 0b11110u || value == 0b11111u;
}

}  // namespace

namespace stm32
{
using namespace utils;

inline void handleMathInstruction(uint16_t opCode, Cpu& cpu)
{
    /// see A5.2.1
    switch (getPart<9, 5>(opCode)) {
        case 0b000'00u ... 0b000'11u:
            switch (getPart<6, 5>(opCode)) {
                case 0b00000u:
                    // see: A7-312
                    return opcodes::cmdMovRegister<opcodes::Encoding::T2>(opCode, cpu);
                default:
                    // see: A7-298
                    return opcodes::cmdShiftImmediate<opcodes::Encoding::T1, ShiftType::LSL>(opCode, cpu);
            }
        case 0b001'00u ... 0b001'11u:
            // see: A7-302
            return opcodes::cmdShiftImmediate<opcodes::Encoding::T1, ShiftType::LSR>(opCode, cpu);
        case 0b010'00u ... 0b010'11u:
            // see: A7-203
            return opcodes::cmdShiftImmediate<opcodes::Encoding::T1, ShiftType::ASR>(opCode, cpu);
        case 0b01100u:
            // see: A7-191
            return opcodes::cmdAddSubRegister<opcodes::Encoding::T1, /* isSub */ false>(opCode, cpu);
        case 0b01101u:
            // see: A7-450
            return opcodes::cmdAddSubRegister<opcodes::Encoding::T1, /* isSub */ true>(opCode, cpu);
        case 0b01110u:
            // see: A7-189
            return opcodes::cmdAddSubImmediate<opcodes::Encoding::T1, /* isSub */ false>(opCode, cpu);
        case 0b01111u:
            // see: A7-448
            return opcodes::cmdAddSubImmediate<opcodes::Encoding::T1, /* isSub */ true>(opCode, cpu);
        case 0b100'00u ... 0b100'11u:
            // see: A7-312
            return opcodes::cmdMovImmediate<opcodes::Encoding::T1>(opCode, cpu);
        case 0b101'00u ... 0b101'11u:
            // see: A7-229
            return opcodes::cmdCmpImmediate<opcodes::Encoding::T1>(opCode, cpu);
        case 0b110'00u ... 0b110'11u:
            // see: A7-189
            return opcodes::cmdAddSubImmediate<opcodes::Encoding::T2, /* isSub */ false>(opCode, cpu);
        case 0b111'00u ... 0b111'11u:
            // see: A7-448
            return opcodes::cmdAddSubImmediate<opcodes::Encoding::T2, /* isSub */ true>(opCode, cpu);
        default:
            UNPREDICTABLE;
    }
}

inline void handleDataProcessingInstruction(uint16_t opCode, Cpu& cpu)
{
    // see A5.2.2
    switch (getPart<6, 4>(opCode)) {
        case 0b0000u:
            // see: A7-201
            return opcodes::cmdBitwiseRegister<opcodes::Encoding::T1, opcodes::Bitwise::AND>(opCode, cpu);
        case 0b0001u:
            // see: A7-239
            return opcodes::cmdBitwiseRegister<opcodes::Encoding::T1, opcodes::Bitwise::EOR>(opCode, cpu);
        case 0b0010u:
            // see: A7-300
            return opcodes::cmdShiftRegister<opcodes::Encoding::T1, ShiftType::LSL>(opCode, cpu);
        case 0b0011u:
            // see: A7-304
            return opcodes::cmdShiftRegister<opcodes::Encoding::T1, ShiftType::LSR>(opCode, cpu);
        case 0b0100u:
            // see: A7-205
            return opcodes::cmdShiftRegister<opcodes::Encoding::T1, ShiftType::ASR>(opCode, cpu);
        case 0b0101u:
            // see: A7-187
            return opcodes::cmdAdcSbcRegister<opcodes::Encoding::T1, /* isSbc */ false>(opCode, cpu);
        case 0b0110u:
            // see: A7-380
            return opcodes::cmdAdcSbcRegister<opcodes::Encoding::T1, /* isSbc */ true>(opCode, cpu);
        case 0b0111u:
            // see: A7-368
            return opcodes::cmdShiftRegister<opcodes::Encoding::T1, ShiftType::ROR>(opCode, cpu);
        case 0b1000u:
            // see: A7-466
            return opcodes::cmdTstRegister<opcodes::Encoding::T1>(opCode, cpu);
        case 0b1001u:
            // see: A7-372
            return opcodes::cmdRsbImmediate<opcodes::Encoding::T1>(opCode, cpu);
        case 0b1010u:
            // see: A7-231
            return opcodes::cmdCmpRegister<opcodes::Encoding::T1, /* isNegative */ false>(opCode, cpu);
        case 0b1011u:
            // see: A7-227
            return opcodes::cmdCmpRegister<opcodes::Encoding::T1, /* isNegative */ true>(opCode, cpu);
        case 0b1100u:
            // see: A7-336
            return opcodes::cmdBitwiseRegister<opcodes::Encoding::T1, opcodes::Bitwise::ORR>(opCode, cpu);
        case 0b1101u:
            // see: A7-234
            return opcodes::cmdMul<opcodes::Encoding::T1>(opCode, cpu);
        case 0b1110u:
            // see: A7-213
            return opcodes::cmdBitwiseRegister<opcodes::Encoding::T1, opcodes::Bitwise::BIC>(opCode, cpu);
        case 0b1111u:
            // see: A7-238
            return opcodes::cmdMvnRegister<opcodes::Encoding::T1>(opCode, cpu);
        default:
            UNPREDICTABLE;
    }
}

inline void handleSpecialDataInstruction(uint16_t opCode, Cpu& cpu)
{
    // see A5.2.3
    switch (getPart<6, 4>(opCode)) {
        case 0b00'00u ... 0b00'11u:
            // see: A7-191
            return opcodes::cmdAddSubRegister<opcodes::Encoding::T2, /* isSub */ false>(opCode, cpu);
        case 0b0101u:
        case 0b011'0u ... 0b011'1u:
            // see: A7-231
            return opcodes::cmdCmpRegister<opcodes::Encoding::T2, /* isNegative */ false>(opCode, cpu);
        case 0b10'00u ... 0b10'11u:
            // see: A7-314
            return opcodes::cmdMovRegister<opcodes::Encoding::T1>(opCode, cpu);
        case 0b110'0u ... 0b110'1u:
            // see: A7-218
            return opcodes::cmdBranchAndExecuteRegister</* withLink */ false>(opCode, cpu);
        case 0b111'0u ... 0b111'1u:
            // see: A7-217
            return opcodes::cmdBranchAndExecuteRegister</* withLink */ true>(opCode, cpu);
        default:
            UNPREDICTABLE;
    }
}

inline void handleLoadFromLiteralPool(uint16_t opCode, Cpu& cpu)
{
    // see: A7.7.43
    opcodes::cmdLoadRegisterLiteral<opcodes::Encoding::T1>(opCode, cpu);
}

inline void handleLoadStoreSingleDataItem(uint16_t opCode, Cpu& /*cpu*/)
{
    // see A5.2.4
    switch (getPart<12, 4>(opCode)) {
        case 0b0101u:
            switch (getPart<9, 3>(opCode)) {
                case 0b000u:
                    // TODO: A7-428
                    return;
                case 0b001u:
                    // TODO: A7-444
                    return;
                case 0b010u:
                    // TODO: A7-432
                    return;
                case 0b011u:
                    // TODO: A7-286
                    return;
                case 0b100u:
                    // TODO: A7-256
                    return;
                case 0b101u:
                    // TODO: A7-278
                    return;
                case 0b110u:
                    // TODO: A7-262
                    return;
                case 0b111u:
                    // TODO: A7-294
                    return;
                default:
                    UNPREDICTABLE;
            }
        case 0b0110u:
            switch (getPart<9, 3>(opCode)) {
                case 0b0'00u ... 0b0'11u:
                    // TODO: A7-426
                    return;
                case 0b1'00u ... 0b1'11u:
                    // TODO: A7-252
                    return;
                default:
                    UNPREDICTABLE;
            }
        case 0b0111u:
            switch (getPart<9, 3>(opCode)) {
                case 0b0'00u ... 0b0'11u:
                    // TODO: A7-430
                    return;
                case 0b1'00u ... 0b1'11u:
                    // TODO: A7-258
                    return;
                default:
                    UNPREDICTABLE;
            }
        case 0b1000u:
            switch (getPart<9, 3>(opCode)) {
                case 0b0'00u ... 0b0'11u:
                    // TODO: A7-442
                    return;
                case 0b1'00u ... 0b1'11u:
                    // TODO: A7-274
                    return;
                default:
                    UNPREDICTABLE;
            }
        case 0b1001u:
            switch (getPart<9, 3>(opCode)) {
                case 0b0'00u ... 0b0'11u:
                    // TODO: A7-426
                    return;
                case 0b1'00u ... 0b1'11u:
                    // TODO: A7-252
                    return;
                default:
                    UNPREDICTABLE;
            }
        default:
            UNPREDICTABLE;
    }
}

inline void handleGeneratePcRelativeAddress(uint16_t opCode, Cpu& cpu)
{
    // see: A7.7.7
    opcodes::cmdAdr<opcodes::Encoding::T1>(opCode, cpu);
}

inline void handleGenerateSpRelativeAddress(uint16_t opCode, Cpu& cpu)
{
    // see: A7.7.5
    opcodes::cmdAddSubSpPlusImmediate<opcodes::Encoding::T1, /*isSub*/ false>(opCode, cpu);
}

inline void handleMiscInstruction(uint16_t opCode, Cpu& cpu)
{
    // see A5.2.5
    switch (getPart<5, 7>(opCode)) {
        case 0b00000'00u ... 0b00000'11u:
            // see: A7-193
            return opcodes::cmdAddSubSpPlusImmediate<opcodes::Encoding::T2, /*isSub*/ false>(opCode, cpu);
        case 0b00001'00u ... 0b00001'11u:
            // see: A7-452
            return opcodes::cmdAddSubSpPlusImmediate<opcodes::Encoding::T1, /*isSub*/ true>(opCode, cpu);
        case 0b0001'000u ... 0b0001'111u:
            // see: A7-219
            return opcodes::cmdCompareAndBranchOnZero(opCode, cpu);
        case 0b001000'0u ... 0b001000'1u:
            // see: A7-461
            return opcodes::cmdExtend<opcodes::Encoding::T1, uint16_t, /*isSignedExtend*/ true>(opCode, cpu);
        case 0b001001'0u ... 0b001001'1u:
            // see: A7-459
            return opcodes::cmdExtend<opcodes::Encoding::T1, uint8_t, /*isSignedExtend*/ true>(opCode, cpu);
        case 0b001010'0u ... 0b001010'1u:
            // see: A7-500
            return opcodes::cmdExtend<opcodes::Encoding::T1, uint16_t, /*isSignedExtend*/ false>(opCode, cpu);
        case 0b001011'0u ... 0b001011'1u:
            // see: A7-498
            return opcodes::cmdExtend<opcodes::Encoding::T1, uint8_t, /*isSignedExtend*/ false>(opCode, cpu);
        case 0b0011'000u ... 0b0011'111u:
            // see: A7-219
            return opcodes::cmdCompareAndBranchOnZero(opCode, cpu);
        case 0b010'0000u ... 0b010'1111u:
            // see: A7-350
            return opcodes::cmdPush<opcodes::Encoding::T1>(opCode, cpu);
        case 0b0110011u:
            // see: B5-731
            return opcodes::cmdCps(opCode, cpu);
        case 0b1001'000u ... 0b1001'111u:
            // see: A7-219
            return opcodes::cmdCompareAndBranchOnZero(opCode, cpu);
        case 0b101000'0u ... 0b101000'1u:
            // see: A7-363
            return opcodes::cmdReverse<opcodes::Encoding::T1, uint32_t>(opCode, cpu);
        case 0b101001'0u ... 0b101001'1u:
            // see: A7-364
            return opcodes::cmdReverse<opcodes::Encoding::T1, uint16_t, /*isSigned*/ false>(opCode, cpu);
        case 0b101011'0u ... 0b101011'1u:
            // see: A7-365
            return opcodes::cmdReverse<opcodes::Encoding::T1, uint16_t, /*isSigned*/ true>(opCode, cpu);
        case 0b1011'000u ... 0b1011'111u:
            // see A7-219
            return opcodes::cmdCompareAndBranchOnZero(opCode, cpu);
        case 0b110'0000u ... 0b110'1111u:
            // see: A7-348
            return opcodes::cmdPop<opcodes::Encoding::T1>(opCode, cpu);
        case 0b1110'000u ... 0b1110'111u:
            // see: A7-215
            return;  // SKIP BECAUSE DEBUG IS UNIMPLEMENTED!
        case 0b1111'000u ... 0b1111'111u:
            // see A5-133
            switch (getPart<0, 4>(opCode)) {
                case 0b0000u:
                    switch (getPart<4, 4>(opCode)) {
                        case 0b0000u:
                            // see: A7-331
                            return opcodes::cmdHint<opcodes::Hint::Nop>(opCode, cpu);
                        case 0b0001u:
                            // see: A7-562
                            return opcodes::cmdHint<opcodes::Hint::Yield>(opCode, cpu);
                        case 0b0010u:
                            // TODO: A7-560
                            return opcodes::cmdHint<opcodes::Hint::WaitForEvent>(opCode, cpu);
                        case 0b0011u:
                            // TODO: A7-561
                            return opcodes::cmdHint<opcodes::Hint::WaitForInterrupt>(opCode, cpu);
                        case 0b0100u:
                            // TODO: A7-385
                            return opcodes::cmdHint<opcodes::Hint::SendEvent>(opCode, cpu);
                        default:
                            return;  // ignore other
                    }
                default:
                    // see: A7-242
                    return opcodes::cmdIfThen(opCode, cpu);
            }
        default:
            UNPREDICTABLE;
    }
}

inline void handleStoreMultipleRegisters(uint16_t opCode, Cpu& cpu)
{
    // see: A7.7.156
    return opcodes::cmdStoreMultiple<opcodes::Encoding::T1>(opCode, cpu);
}

inline void handleLoadMultipleRegisters(uint16_t opCode, Cpu& cpu)
{
    // see: A7.7.40
    return opcodes::cmdLoadMultiple<opcodes::Encoding::T1>(opCode, cpu);
}

inline void handleConditionalBranch(uint16_t opCode, Cpu& cpu)
{
    // see A5.2.6
    switch (getPart<8, 4>(opCode)) {
        case 0b1110u:
            // see: A7-471
            return opcodes::cmdPermanentlyUndefined<opcodes::Encoding::T1>(opCode, cpu);
        case 0b1111u:
            // see: A7-455
            return opcodes::cmdCallSupervisor(opCode, cpu);
        default:
            // see: A7-207
            return opcodes::cmdBranch<opcodes::Encoding::T1>(opCode, cpu);
    }
}

inline void handleUnconditionalBranch(uint16_t opCode, Cpu& cpu)
{
    // see: A7.7.12
    opcodes::cmdBranch<opcodes::Encoding::T2>(opCode, cpu);
}

void Cpu::step()
{
    auto& PC = m_registers.PC();
    m_currentInstructionAddress = PC;

    const auto opCodeHw1 = m_memory.read<uint16_t>(PC & ~RIGHT_BIT<uint32_t>);

    if (is32bitInstruction(opCodeHw1)) {
        /*
        const auto opCodeHw2Low = m_memory.read(PC & ~math::RIGHT_BIT<uint32_t>);
        const auto opCodeHw2High = m_memory.read(PC & ~math::RIGHT_BIT<uint32_t> + 1);
        PC += 2u;

        const auto opCode = math::combine<uint32_t>(math::Part<0, 8>{opCodeHw1Low},
                                                    math::Part<8, 8>{opCodeHw1High},
                                                    math::Part<16, 8>{opCodeHw2Low},
                                                    math::Part<24, 8>{opCodeHw2High});
        */
        // TODO: handle 32 bit instructions
    }
    else {
        switch (getPart<2, 6>(opCodeHw1)) {
            case 0b00'0000u ... 0b00'1111u:
                handleMathInstruction(opCodeHw1, *this);
                break;
            case 0b010000u:
                handleDataProcessingInstruction(opCodeHw1, *this);
                break;
            case 0b010001u:
                handleSpecialDataInstruction(opCodeHw1, *this);
                break;
            case 0b01001'0u ... 0b01001'1u:
                handleLoadFromLiteralPool(opCodeHw1, *this);
                break;
            case 0b0101'00u ... 0b0101'11u:
            case 0b011'000u ... 0b011'111u:
            case 0b100'000u ... 0b100'111u:
                handleLoadStoreSingleDataItem(opCodeHw1, *this);
                break;
            case 0b10100'0u ... 0b10100'1u:
                handleGeneratePcRelativeAddress(opCodeHw1, *this);
                break;
            case 0b10101'0u ... 0b10101'1u:
                handleGenerateSpRelativeAddress(opCodeHw1, *this);
                break;
            case 0b1011'00u ... 0b1011'11u:
                handleMiscInstruction(opCodeHw1, *this);
                break;
            case 0b11000'0u ... 0b11000'1u:
                handleStoreMultipleRegisters(opCodeHw1, *this);
                break;
            case 0b11001'0u ... 0b11001'1u:
                handleLoadMultipleRegisters(opCodeHw1, *this);
                break;
            case 0b1101'00u ... 0b1101'11u:
                handleConditionalBranch(opCodeHw1, *this);
                break;
            case 0b11100'0u ... 0b11100'0u:
                handleUnconditionalBranch(opCodeHw1, *this);
                break;
            default:
                UNPREDICTABLE;
        }

        PC += 2u;
    }
}

}  // namespace stm32
