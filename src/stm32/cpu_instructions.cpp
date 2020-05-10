// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "cpu.hpp"
#include "opcodes.hpp"

namespace stm32
{
using namespace utils;

namespace hw
{
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
    opcodes::cmdLoadLiteral<opcodes::Encoding::T1>(opCode, cpu);
}

inline void handleLoadStoreSingleDataItem(uint16_t opCode, Cpu& cpu)
{
    // see A5.2.4
    switch (getPart<12, 4>(opCode)) {
        case 0b0101u:
            switch (getPart<9, 3>(opCode)) {
                case 0b000u:
                    // see: A7-428
                    return opcodes::cmdStoreRegister<opcodes::Encoding::T1, uint32_t>(opCode, cpu);
                case 0b001u:
                    // see: A7-444
                    return opcodes::cmdStoreRegister<opcodes::Encoding::T1, uint16_t>(opCode, cpu);
                case 0b010u:
                    // see: A7-432
                    return opcodes::cmdStoreRegister<opcodes::Encoding::T1, uint8_t>(opCode, cpu);
                case 0b011u:
                    // see: A7-286
                    return opcodes::cmdLoadRegister<opcodes::Encoding::T1, uint8_t, /*isSignExtended*/ true>(opCode, cpu);
                case 0b100u:
                    // see: A7-256
                    return opcodes::cmdLoadRegister<opcodes::Encoding::T1, uint32_t>(opCode, cpu);
                case 0b101u:
                    // see: A7-278
                    return opcodes::cmdLoadRegister<opcodes::Encoding::T1, uint16_t>(opCode, cpu);
                case 0b110u:
                    // see: A7-262
                    return opcodes::cmdLoadRegister<opcodes::Encoding::T1, uint8_t>(opCode, cpu);
                case 0b111u:
                    // see: A7-294
                    return opcodes::cmdLoadRegister<opcodes::Encoding::T1, uint16_t, /*isSignExtended*/ true>(opCode, cpu);
                default:
                    UNPREDICTABLE;
            }
        case 0b0110u:
            switch (getPart<9, 3>(opCode)) {
                case 0b0'00u ... 0b0'11u:
                    // see: A7-426
                    return opcodes::cmdStoreImmediate<opcodes::Encoding::T1, uint32_t>(opCode, cpu);
                case 0b1'00u ... 0b1'11u:
                    // see: A7-252
                    return opcodes::cmdLoadImmediate<opcodes::Encoding::T1, uint32_t>(opCode, cpu);
                default:
                    UNPREDICTABLE;
            }
        case 0b0111u:
            switch (getPart<9, 3>(opCode)) {
                case 0b0'00u ... 0b0'11u:
                    // see: A7-430
                    return opcodes::cmdStoreImmediate<opcodes::Encoding::T1, uint8_t>(opCode, cpu);
                case 0b1'00u ... 0b1'11u:
                    // see: A7-258
                    return opcodes::cmdLoadImmediate<opcodes::Encoding::T1, uint8_t>(opCode, cpu);
                default:
                    UNPREDICTABLE;
            }
        case 0b1000u:
            switch (getPart<9, 3>(opCode)) {
                case 0b0'00u ... 0b0'11u:
                    // see: A7-442
                    return opcodes::cmdStoreImmediate<opcodes::Encoding::T1, uint16_t>(opCode, cpu);
                case 0b1'00u ... 0b1'11u:
                    // see: A7-274
                    return opcodes::cmdLoadImmediate<opcodes::Encoding::T1, uint16_t>(opCode, cpu);
                default:
                    UNPREDICTABLE;
            }
        case 0b1001u:
            switch (getPart<9, 3>(opCode)) {
                case 0b0'00u ... 0b0'11u:
                    // see: A7-426
                    return opcodes::cmdStoreImmediate<opcodes::Encoding::T2, uint32_t>(opCode, cpu);
                case 0b1'00u ... 0b1'11u:
                    // see: A7-252
                    return opcodes::cmdLoadImmediate<opcodes::Encoding::T2, uint32_t>(opCode, cpu);
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
            return opcodes::cmdExtend<opcodes::Encoding::T1, uint16_t, /*isSignExtended*/ true>(opCode, cpu);
        case 0b001001'0u ... 0b001001'1u:
            // see: A7-459
            return opcodes::cmdExtend<opcodes::Encoding::T1, uint8_t, /*isSignExtended*/ true>(opCode, cpu);
        case 0b001010'0u ... 0b001010'1u:
            // see: A7-500
            return opcodes::cmdExtend<opcodes::Encoding::T1, uint16_t, /*isSignExtended*/ false>(opCode, cpu);
        case 0b001011'0u ... 0b001011'1u:
            // see: A7-498
            return opcodes::cmdExtend<opcodes::Encoding::T1, uint8_t, /*isSignExtended*/ false>(opCode, cpu);
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
            return opcodes::cmdReverse<opcodes::Encoding::T1, uint16_t, /*isSignExtended*/ false>(opCode, cpu);
        case 0b101011'0u ... 0b101011'1u:
            // see: A7-365
            return opcodes::cmdReverse<opcodes::Encoding::T1, uint16_t, /*isSignExtended*/ true>(opCode, cpu);
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
                            // see: A7-560
                            return opcodes::cmdHint<opcodes::Hint::WaitForEvent>(opCode, cpu);
                        case 0b0011u:
                            // see: A7-561
                            return opcodes::cmdHint<opcodes::Hint::WaitForInterrupt>(opCode, cpu);
                        case 0b0100u:
                            // see: A7-385
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
}  // namespace hw

namespace wo
{
inline void loadMultipleAndStoreMultiple(uint32_t opCode, Cpu& cpu)
{
    const auto [Rn, L, W, op] = split<uint32_t, _<16, 4>, _<20, 1>, _<21, 1>, _<23, 2>>(opCode);

    // see: A5-142
    switch (op) {
        case 0b01u:
            if (L == 0) {
                // see: A7-422
                return opcodes::cmdStoreMultiple<opcodes::Encoding::T2>(opCode, cpu);
            }
            else {
                if (W != 1u && Rn != 0b1101u) {
                    // see: A7-248
                    return opcodes::cmdLoadMultiple<opcodes::Encoding::T2>(opCode, cpu);
                }
                else {
                    // see: A7-348
                    return opcodes::cmdPop<opcodes::Encoding::T2>(opCode, cpu);
                }
            }
        case 0b10u:
            if (L == 0) {
                if (W != 1u || Rn != 0b1101u) {
                    // TODO: A7-424
                    return;
                }
                else {
                    // see: A7-350
                    return opcodes::cmdPush<opcodes::Encoding::T2>(opCode, cpu);
                }
            }
            else {
                // TODO: A7-250
                return;
            }
        default:
            UNPREDICTABLE;
    }
}

inline void loadStoreDualOrExclusive(uint32_t /*opCode*/, Cpu& /*cpu*/)
{
    // TODO: A5-143
}

inline void dataProcessingShiftedRegister(uint32_t opCode, Cpu& cpu)
{
    const auto [Rd, Rn, S, op] = split<uint32_t, _<8, 4>, _<16, 4>, _<20, 1>, _<21, 4>>(opCode);

    // see: A5-148
    switch (op) {
        case 0b0000u:
            if (Rd != 0b1111u) {
                // see: A7-201
                return opcodes::cmdBitwiseRegister<opcodes::Encoding::T2, opcodes::Bitwise::AND>(opCode, cpu);
            }
            else if (S) {
                // see: A7-466
                return opcodes::cmdTstRegister<opcodes::Encoding::T2>(opCode, cpu);
            }
            break;
        case 0b0001u:
            // see: A7-213
            return opcodes::cmdBitwiseRegister<opcodes::Encoding::T2, opcodes::Bitwise::BIC>(opCode, cpu);
        case 0b0010u:
            if (Rn != 0b1111u) {
                // seeL A7-336
                return opcodes::cmdBitwiseRegister<opcodes::Encoding::T2, opcodes::Bitwise::ORR>(opCode, cpu);
            }
            else {
                const auto [type, imm2, imm3] = split<uint32_t, _<4, 2>, _<6, 2>, _<12, 3>>(opCode);

                switch (type) {
                    case 0b00u:
                        if (imm2 == 0u && imm3 == 0u) {
                            // see: A7-314
                            return opcodes::cmdMovRegister<opcodes::Encoding::T3>(opCode, cpu);
                        }
                        else {
                            // see: A7-298
                            return opcodes::cmdShiftImmediate<opcodes::Encoding::T2, ShiftType::LSL>(opCode, cpu);
                        }
                    case 0b01u:
                        // see: A7-302
                        return opcodes::cmdShiftImmediate<opcodes::Encoding::T2, ShiftType::LSR>(opCode, cpu);
                    case 0b10u:
                        // see: A7-203
                        return opcodes::cmdShiftImmediate<opcodes::Encoding::T2, ShiftType::ASR>(opCode, cpu);
                    case 0b11u:
                        if (imm2 == 0u && imm3 == 0u) {
                            // TODO: A7-370
                            return;
                        }
                        else {
                            // TODO: A7-336
                            return;
                        }
                    default:
                        UNPREDICTABLE;
                }
            }
        case 0b0011u:
            if (Rn != 0b1111u) {
                // TODO: A7-333
                return;
            }
            else {
                // see: A7-238
                return opcodes::cmdMvnRegister<opcodes::Encoding::T2>(opCode, cpu);
            }
        case 0b0100u:
            if (Rd != 0b1111u) {
                // see: A7-239
                return opcodes::cmdBitwiseRegister<opcodes::Encoding::T2, opcodes::Bitwise::EOR>(opCode, cpu);
            }
            else if (S) {
                // TODO: A7-464
                return;
            }
            break;
        case 0b1000u:
            if (Rd != 0b1111u) {
                // see: A7-191
                return opcodes::cmdAddSubRegister<opcodes::Encoding::T3, /*isSub*/ false>(opCode, cpu);
            }
            else if (S) {
                // see: A7-227
                return opcodes::cmdCmpRegister<opcodes::Encoding::T2, /*isNegative*/ true>(opCode, cpu);
            }
            break;
        case 0b1010u:
            // see: A7-187
            return opcodes::cmdAdcSbcRegister<opcodes::Encoding::T2, /*isSbc*/ false>(opCode, cpu);
        case 0b1011u:
            // see: A7-380
            return opcodes::cmdAdcSbcRegister<opcodes::Encoding::T2, /*isSbc*/ true>(opCode, cpu);
        case 0b1101u:
            if (Rd != 0b1111u) {
                // see: A7-450
                return opcodes::cmdAddSubRegister<opcodes::Encoding::T2, /*isSub*/ true>(opCode, cpu);
            }
            else if (S) {
                // see: A7-231
                return opcodes::cmdCmpRegister<opcodes::Encoding::T3, /*isNegative*/ false>(opCode, cpu);
            }
            break;
        case 0b1110u:
            // TODO: A7-374
            return;
        default:
            break;
    }

    UNPREDICTABLE;
}

inline void coprocessorInstructions(uint32_t /*opCode*/, Cpu& /*cpu*/)
{
    // TODO: A5-156
}

inline void dataProcessingModifiedImmediate(uint32_t opCode, Cpu& cpu)
{
    const auto [Rd, Rn, op] = split<uint32_t, _<8, 4>, _<16, 4>, _<20, 6>>(opCode);

    // see: A5-136
    switch (op) {
        case 0b0000'0u ... 0b0000'1u:
            if (Rd != 0b1111u) {
                // TODO: A7-199
                return;
            }
            else {
                // TODO: A7-465
                return;
            }
        case 0b0001'0u ... 0b0001'1u:
            // TODO: A7-211
            return;
        case 0b0010'0u ... 0b0010'1u:
            if (Rn != 0b1111u) {
                // TODO: A7-334
                return;
            }
            else {
                // see: A7-312
                return opcodes::cmdMovImmediate<opcodes::Encoding::T2>(opCode, cpu);
            }
        case 0b0011'0u ... 0b0011'1u:
            if (Rn != 0b1111u) {
                // TODO: A7-332
                return;
            }
            else {
                // TODO: A7-326
                return;
            }
        case 0b0100'0u ... 0b0100'1u:
            if (Rd != 0b1111u) {
                // TODO: A7-238
                return;
            }
            else {
                // TODO: A7-463
                return;
            }
        case 0b1000'0u ... 0b1000'1u:
            if (Rd != 0b1111) {
                // see: A7-189
                return opcodes::cmdAddSubImmediate<opcodes::Encoding::T3, /*isSub*/ false>(opCode, cpu);
            }
            else {
                // TODO: A7-225
                return;
            }
        case 0b1010'0u ... 0b1010'1u:
            // TODO: A7-185
            return;
        case 0b1011'0u ... 0b1011'1u:
            // TODO: A7-379
            return;
        case 0b1101'0u ... 0b1101'1u:
            if (Rd != 0b1111u) {
                // see: A7-448
                return opcodes::cmdAddSubImmediate<opcodes::Encoding::T3, /*isSub*/ true>(opCode, cpu);
            }
            else {
                // see: A7-229
                return opcodes::cmdCmpImmediate<opcodes::Encoding::T2>(opCode, cpu);
            }
        case 0b1110'0u ... 0b1110'1u:
            // see: A7-372
            return opcodes::cmdRsbImmediate<opcodes::Encoding::T2>(opCode, cpu);
        default:
            UNPREDICTABLE;
    }
}

inline void dataProcessingPlainBinaryImmediate(uint32_t /*opCode*/, Cpu& /*cpu*/)
{
    // TODO: A5-139
}

inline void branchesAndMiscControl(uint32_t /*opCode*/, Cpu& /*cpu*/)
{
    // TODO: A5-140
}

inline void storeSingleDataItem(uint32_t opCode, Cpu& cpu)
{
    const auto [op2, op1] = split<uint32_t, _<6, 6>, _<21, 3>>(opCode);

    // see: A5-147
    switch (op1) {
        case 0b000u:
            if (isBitSet<5>(op2)) {
                // see: A-430
                return opcodes::cmdStoreImmediate<opcodes::Encoding::T3, uint8_t>(opCode, cpu);
            }
            else {
                // see: A-432
                return opcodes::cmdStoreRegister<opcodes::Encoding::T2, uint8_t>(opCode, cpu);
            }
        case 0b001u:
            if (isBitSet<5>(op2)) {
                // see: A-442
                return opcodes::cmdStoreImmediate<opcodes::Encoding::T3, uint16_t>(opCode, cpu);
            }
            else {
                // see: A-444
                return opcodes::cmdStoreRegister<opcodes::Encoding::T2, uint16_t>(opCode, cpu);
            }
        case 0b010u:
            if (isBitSet<5>(op2)) {
                // see: A-426
                return opcodes::cmdStoreImmediate<opcodes::Encoding::T4, uint32_t>(opCode, cpu);
            }
            else {
                // see: A7-248
                return opcodes::cmdStoreRegister<opcodes::Encoding::T2, uint32_t>(opCode, cpu);
            }
        case 0b100u:
            // see: A-430
            return opcodes::cmdStoreImmediate<opcodes::Encoding::T2, uint8_t>(opCode, cpu);
        case 0b101u:
            // see: A-442
            return opcodes::cmdStoreImmediate<opcodes::Encoding::T2, uint16_t>(opCode, cpu);
        case 0b110u:
            // see: A-426
            return opcodes::cmdStoreImmediate<opcodes::Encoding::T3, uint32_t>(opCode, cpu);
        default:
            UNPREDICTABLE;
    }
}

inline void loadByteAndMemoryHints(uint32_t /*opCode*/, Cpu& /*cpu*/)
{
    // TODO: A5-146
}

inline void loadHalfwordAndMemoryHints(uint32_t /*opCode*/, Cpu& /*cpu*/)
{
    // TODO: A5-145
}

inline void loadWord(uint32_t opCode, Cpu& cpu)
{
    const auto [op2, Rn, op1] = split<uint32_t, _<6, 6>, _<16, 4>, _<23, 2>>(opCode);

    // see: A5-144
    if (Rn != 0b1111u) {
        switch (op1) {
            case 0b01u:
                // see: A7-252
                return opcodes::cmdLoadImmediate<opcodes::Encoding::T3, uint32_t>(opCode, cpu);
            case 0b00u:
                if (op2 & 0b100100u || getPart<2, 4>(op2) == 0b1100u) {
                    // see: A7-252
                    return opcodes::cmdLoadImmediate<opcodes::Encoding::T4, uint32_t>(opCode, cpu);
                }
                else if (getPart<2, 4>(op2) == 0b1110u) {
                    // TODO: A7-256
                    return;
                }
                else if (op2 == 0u) {
                    // see: A7-256
                    return opcodes::cmdLoadRegister<opcodes::Encoding::T2, uint32_t>(opCode, cpu);
                }
                break;
            default:
                break;
        }
    }
    else {
        if (isBitSet<1>(op1)) {
            // see: A7-254
            return opcodes::cmdLoadLiteral<opcodes::Encoding::T2>(opCode, cpu);
        }
    }

    UNPREDICTABLE;
}

inline void dataProcessingRegister(uint32_t opCode, Cpu& cpu)
{
    const auto [op2, Rn, op1] = split<uint32_t, _<4, 4>, _<16, 4>, _<20, 4>>(opCode);

    // see: A5-150
    switch (op1) {
        case 0b000'0u ... 0b000'1u:
            if (op2 == 0b0000u) {
                // see: A7-300
                return opcodes::cmdShiftRegister<opcodes::Encoding::T2, ShiftType::LSL>(opCode, cpu);
            }
            else if (isBitSet<3>(op2) && Rn == 0b1111u) {
                if (isBitClear<0>(op1)) {
                    // see: A7-461
                    return opcodes::cmdExtend<opcodes::Encoding::T2, uint16_t, /*isSignExtended*/ true>(opCode, cpu);
                }
                else {
                    // see: A7-500
                    return opcodes::cmdExtend<opcodes::Encoding::T2, uint16_t, /*isSignExtended*/ false>(opCode, cpu);
                }
            }
            break;
        case 0b001'0u ... 0b001'1u:
            if (op2 == 0b0000u) {
                // see: a7-304
                return opcodes::cmdShiftRegister<opcodes::Encoding::T2, ShiftType::LSR>(opCode, cpu);
            }
            break;
        case 0b010'0u ... 0b010'1u:
            if (op2 == 0b0000u) {
                // see: A7-205
                return opcodes::cmdShiftRegister<opcodes::Encoding::T2, ShiftType::ASR>(opCode, cpu);
            }
            else if (isBitSet<3>(op2) && Rn == 0b1111u) {
                if (isBitClear<0>(op1)) {
                    // see: A7-459
                    return opcodes::cmdExtend<opcodes::Encoding::T2, uint8_t, /*isSignExtended*/ true>(opCode, cpu);
                }
                else {
                    // see: A7-498
                    return opcodes::cmdExtend<opcodes::Encoding::T2, uint8_t, /*isSignExtended*/ false>(opCode, cpu);
                }
            }
            break;
        case 0b011'0u ... 0b011'1u:
            if (op2 == 0b0000u) {
                // see: A7-368
                return opcodes::cmdShiftRegister<opcodes::Encoding::T2, ShiftType::ROR>(opCode, cpu);
            }
            break;
        case 0b1001u:
            switch (op2) {
                case 0b1000u:
                    // see: A7-363
                    return opcodes::cmdReverse<opcodes::Encoding::T2, uint32_t>(opCode, cpu);
                case 0b1001u:
                    // see: A7-364
                    return opcodes::cmdReverse<opcodes::Encoding::T2, uint16_t>(opCode, cpu);
                case 0b1010u:
                    // TODO: A7-362
                    return;
                case 0b1011u:
                    // see: A7-365
                    return opcodes::cmdReverse<opcodes::Encoding::T2, uint16_t, /*isSignExtended*/ true>(opCode, cpu);
                default:
                    break;
            }
            break;
        case 0b1011u:
            if (op2 == 0b1000u) {
                // TODO: A7-224
                return;
            }
            break;
        default:
            break;
    }

    UNPREDICTABLE;
}

inline void multiplicationAndAbsoluteDifference(uint32_t /*opCode*/, Cpu& /*cpu*/)
{
    // TODO: A5-154
}

inline void longMultiplicationAndDivision(uint32_t /*opCode*/, Cpu& /*cpu*/)
{
    // TODO: A5-154
}

}  // namespace wo

void Cpu::step()
{
    auto& PC = m_registers.PC();
    m_currentInstructionAddress = PC;

    const auto opCodeHw1 = m_memory.read<uint16_t>(PC & ~RIGHT_BIT<uint32_t>);

    const auto op1 = getPart<11, 2>(opCodeHw1);

    if (op1 == 0u) {
        switch (getPart<2, 6>(opCodeHw1)) {
            case 0b00'0000u ... 0b00'1111u:
                hw::handleMathInstruction(opCodeHw1, *this);
                break;
            case 0b010000u:
                hw::handleDataProcessingInstruction(opCodeHw1, *this);
                break;
            case 0b010001u:
                hw::handleSpecialDataInstruction(opCodeHw1, *this);
                break;
            case 0b01001'0u ... 0b01001'1u:
                hw::handleLoadFromLiteralPool(opCodeHw1, *this);
                break;
            case 0b0101'00u ... 0b0101'11u:
            case 0b011'000u ... 0b011'111u:
            case 0b100'000u ... 0b100'111u:
                hw::handleLoadStoreSingleDataItem(opCodeHw1, *this);
                break;
            case 0b10100'0u ... 0b10100'1u:
                hw::handleGeneratePcRelativeAddress(opCodeHw1, *this);
                break;
            case 0b10101'0u ... 0b10101'1u:
                hw::handleGenerateSpRelativeAddress(opCodeHw1, *this);
                break;
            case 0b1011'00u ... 0b1011'11u:
                hw::handleMiscInstruction(opCodeHw1, *this);
                break;
            case 0b11000'0u ... 0b11000'1u:
                hw::handleStoreMultipleRegisters(opCodeHw1, *this);
                break;
            case 0b11001'0u ... 0b11001'1u:
                hw::handleLoadMultipleRegisters(opCodeHw1, *this);
                break;
            case 0b1101'00u ... 0b1101'11u:
                hw::handleConditionalBranch(opCodeHw1, *this);
                break;
            case 0b11100'0u ... 0b11100'0u:
                hw::handleUnconditionalBranch(opCodeHw1, *this);
                break;
            default:
                UNPREDICTABLE;
        }

        PC += 2u;
    }
    else {
        const auto opCodeHw2 = m_memory.read<uint16_t>((PC & ~RIGHT_BIT<uint32_t>)+2u);
        const auto opCode = combine<uint32_t>(_<0, 16, uint16_t>{opCodeHw2}, _<16, 16, uint16_t>{opCodeHw1});

        const auto op2 = getPart<4, 7>(opCodeHw1);

        switch (op1) {
            case 0b01u:
                switch (op2) {
                    case 0b00000'00u ... 0b00000'11u:
                    case 0b00010'00u ... 0b00010'11u:
                    case 0b00100'00u ... 0b00100'11u:
                    case 0b00110'00u ... 0b00110'11u:
                        wo::loadMultipleAndStoreMultiple(opCode, *this);
                        break;
                    case 0b00001'00u ... 0b00001'11u:
                    case 0b00011'00u ... 0b00011'11u:
                    case 0b00101'00u ... 0b00101'11u:
                    case 0b00111'00u ... 0b00111'11u:
                        wo::loadStoreDualOrExclusive(opCode, *this);
                        break;
                    case 0b01'00000u ... 0b01'11111u:
                        wo::dataProcessingShiftedRegister(opCode, *this);
                        break;
                    case 0b1'000000u ... 0b1'111111u:
                        wo::coprocessorInstructions(opCode, *this);
                        break;
                    default:
                        UNPREDICTABLE;
                }
                break;
            case 0b10u:
                switch (op2) {
                    case 0b00'00000u ... 0b00'11111u:
                    case 0b10'00000u ... 0b10'11111u:
                        UNPREDICTABLE_IF((!isBitClear<15>(opCodeHw2)));
                        wo::dataProcessingModifiedImmediate(opCode, *this);
                        break;
                    case 0b01'00000u ... 0b01'11111u:
                    case 0b11'00000u ... 0b11'11111u:
                        UNPREDICTABLE_IF((!isBitClear<15>(opCodeHw2)));
                        wo::dataProcessingPlainBinaryImmediate(opCode, *this);
                        break;
                    default:
                        UNPREDICTABLE_IF((!isBitSet<15>(opCodeHw2)));
                        wo::branchesAndMiscControl(opCode, *this);
                        break;
                }
                break;
            case 0b11u:
                switch (op2) {
                    case 0b00'00000u ... 0b00'11111u:
                        if ((op2 & 0b0001110u) == 0u) {
                            wo::storeSingleDataItem(opCode, *this);
                        }
                        else {
                            switch (getPart<0, 3>(op2)) {
                                case 0b001u:
                                    wo::loadByteAndMemoryHints(opCode, *this);
                                    break;
                                case 0b011u:
                                    wo::loadHalfwordAndMemoryHints(opCode, *this);
                                    break;
                                case 0b101u:
                                    wo::loadWord(opCode, *this);
                                    break;
                                default:
                                    UNPREDICTABLE;
                            }
                        }
                        break;
                    case 0b010'0000u ... 0b010'1111u:
                        wo::dataProcessingRegister(opCode, *this);
                        break;
                    case 0b0110'000u ... 0b0110'111u:
                        wo::multiplicationAndAbsoluteDifference(opCode, *this);
                        break;
                    case 0b0111'000u ... 0b0111'111u:
                        wo::longMultiplicationAndDivision(opCode, *this);
                        break;
                    case 0b1'000000u ... 0b1'111111u:
                        wo::coprocessorInstructions(opCode, *this);
                        break;
                    default:
                        UNPREDICTABLE;
                }
                break;
            default:
                UNPREDICTABLE;
        }

        PC += 4u;
    }
}  // namespace wo

}  // namespace stm32
