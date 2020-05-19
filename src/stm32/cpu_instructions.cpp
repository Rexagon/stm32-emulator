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
            return opcodes::cmdCmpImmediate<opcodes::Encoding::T1, /*isNegative*/ false>(opCode, cpu);
        case 0b110'00u ... 0b110'11u:
            // see: A7-189
            return opcodes::cmdAddSubImmediate<opcodes::Encoding::T2, /* isSub */ false>(opCode, cpu);
        case 0b111'00u ... 0b111'11u:
            // see: A7-448
            return opcodes::cmdAddSubImmediate<opcodes::Encoding::T2, /* isSub */ true>(opCode, cpu);
        default:
            UNDEFINED;
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
            UNDEFINED;
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
            UNDEFINED;
    }
}

inline void handleLoadFromLiteralPool(uint16_t opCode, Cpu& cpu)
{
    // see: A7.7.43
    opcodes::cmdLoadLiteral<opcodes::Encoding::T1, uint32_t, /*isSignExtended*/ false>(opCode, cpu);
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
                    UNDEFINED;
            }
        case 0b0110u:
            switch (getPart<9, 3>(opCode)) {
                case 0b0'00u ... 0b0'11u:
                    // see: A7-426
                    return opcodes::cmdStoreImmediate<opcodes::Encoding::T1, uint32_t>(opCode, cpu);
                case 0b1'00u ... 0b1'11u:
                    // see: A7-252
                    return opcodes::cmdLoadImmediate<opcodes::Encoding::T1, uint32_t, /*isSignExtended*/ false>(opCode, cpu);
                default:
                    UNDEFINED;
            }
        case 0b0111u:
            switch (getPart<9, 3>(opCode)) {
                case 0b0'00u ... 0b0'11u:
                    // see: A7-430
                    return opcodes::cmdStoreImmediate<opcodes::Encoding::T1, uint8_t>(opCode, cpu);
                case 0b1'00u ... 0b1'11u:
                    // see: A7-258
                    return opcodes::cmdLoadImmediate<opcodes::Encoding::T1, uint8_t, /*isSignExtended*/ false>(opCode, cpu);
                default:
                    UNDEFINED;
            }
        case 0b1000u:
            switch (getPart<9, 3>(opCode)) {
                case 0b0'00u ... 0b0'11u:
                    // see: A7-442
                    return opcodes::cmdStoreImmediate<opcodes::Encoding::T1, uint16_t>(opCode, cpu);
                case 0b1'00u ... 0b1'11u:
                    // see: A7-274
                    return opcodes::cmdLoadImmediate<opcodes::Encoding::T1, uint16_t, /*isSignExtended*/ false>(opCode, cpu);
                default:
                    UNDEFINED;
            }
        case 0b1001u:
            switch (getPart<9, 3>(opCode)) {
                case 0b0'00u ... 0b0'11u:
                    // see: A7-426
                    return opcodes::cmdStoreImmediate<opcodes::Encoding::T2, uint32_t>(opCode, cpu);
                case 0b1'00u ... 0b1'11u:
                    // see: A7-252
                    return opcodes::cmdLoadImmediate<opcodes::Encoding::T2, uint32_t, /*isSignExtended*/ false>(opCode, cpu);
                default:
                    UNDEFINED;
            }
        default:
            UNDEFINED;
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
            return opcodes::cmdReverseBytes<opcodes::Encoding::T1, uint32_t>(opCode, cpu);
        case 0b101001'0u ... 0b101001'1u:
            // see: A7-364
            return opcodes::cmdReverseBytes<opcodes::Encoding::T1, uint16_t, /*isSignExtended*/ false>(opCode, cpu);
        case 0b101011'0u ... 0b101011'1u:
            // see: A7-365
            return opcodes::cmdReverseBytes<opcodes::Encoding::T1, uint16_t, /*isSignExtended*/ true>(opCode, cpu);
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
            UNDEFINED;
    }
}

inline void handleStoreMultipleRegisters(uint16_t opCode, Cpu& cpu)
{
    // see: A7.7.156
    return opcodes::cmdStoreMultipleIncrementAfter<opcodes::Encoding::T1>(opCode, cpu);
}

inline void handleLoadMultipleRegisters(uint16_t opCode, Cpu& cpu)
{
    // see: A7.7.40
    return opcodes::cmdLoadMultipleIncrementAfter<opcodes::Encoding::T1>(opCode, cpu);
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
    const auto [Rn, L, W, op] = split<_<16, 4>, _<20>, _<21>, _<23, 2>>(opCode);

    // see: A5-142
    switch (op) {
        case 0b01u:
            if (L == 0) {
                // see: A7-422
                return opcodes::cmdStoreMultipleIncrementAfter<opcodes::Encoding::T2>(opCode, cpu);
            }
            else {
                if (W != 1u && Rn != 0b1101u) {
                    // see: A7-248
                    return opcodes::cmdLoadMultipleIncrementAfter<opcodes::Encoding::T2>(opCode, cpu);
                }
                else {
                    // see: A7-348
                    return opcodes::cmdPop<opcodes::Encoding::T2>(opCode, cpu);
                }
            }
        case 0b10u:
            if (L == 0) {
                if (W != 1u || Rn != 0b1101u) {
                    // see: A7-424
                    return opcodes::cmdStoreMultipleDecrementBefore(opCode, cpu);
                }
                else {
                    // see: A7-350
                    return opcodes::cmdPush<opcodes::Encoding::T2>(opCode, cpu);
                }
            }
            else {
                // see: A7-250
                return opcodes::cmdLoadMultipleDecrementBefore(opCode, cpu);
            }
        default:
            UNDEFINED;
    }
}

inline void loadStoreDualOrExclusive(uint32_t opCode, Cpu& cpu)
{
    const auto [op3, op2, op1] = split<_<4, 4>, _<20, 2>, _<23, 2>>(opCode);

    // see: A5-143
    if (op1 == 0b00u && op2 == 0b00u) {
        // see: A7-438
        UNIMPLEMENTED;
    }
    if (op1 == 0b00u && op2 == 0b01u) {
        // see: A7-270
        UNIMPLEMENTED;
    }
    if ((op2 == 0b10u && isBitClear<1>(op1)) || (isBitClear<0>(op2) && isBitSet<1>(op1))) {
        // see: A7-436
        return opcodes::cmdStoreRegisterDual(opCode, cpu);
    }
    if ((op2 == 0b11u && isBitClear<1>(op1)) || (isBitSet<0>(op1) && isBitSet<1>(op1))) {
        // see: A7-266 / A7-268
        return opcodes::cmdLoadRegisterDual(opCode, cpu);
    }
    if (op1 == 0b01u && op2 == 0b00u) {
        switch (op3) {
            case 0b0100u:
                // see: A7-439
                UNIMPLEMENTED;
            case 0b0101u:
                // see: A7-440
                UNIMPLEMENTED;
            default:
                break;
        }
    }
    if (op1 == 0b01u && op2 == 0b01u) {
        switch (op3) {
            case 0b0000u:
                // see: A7-462
                return opcodes::cmdTableBranch<uint8_t>(opCode, cpu);
            case 0b0001u:
                // see: A7-462
                return opcodes::cmdTableBranch<uint16_t>(opCode, cpu);
            case 0b0100u:
                // see: A7-271
                UNIMPLEMENTED;
            case 0b0101u:
                // see: A7-272
                UNIMPLEMENTED;
            default:
                break;
        }
    }

    UNDEFINED;
}

inline void dataProcessingShiftedRegister(uint32_t opCode, Cpu& cpu)
{
    const auto [Rd, Rn, S, op] = split<_<8, 4>, _<16, 4>, _<20>, _<21, 4>>(opCode);

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
                const auto [type, imm2, imm3] = split<_<4, 2>, _<6, 2>, _<12, 3>>(opCode);

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
                            // see: A7-370
                            return opcodes::cmdRrxImmediate(opCode, cpu);
                        }
                        else {
                            // see: A7-366
                            return opcodes::cmdRorImmediate(opCode, cpu);
                        }
                    default:
                        UNDEFINED;
                }
            }
        case 0b0011u:
            if (Rn != 0b1111u) {
                // see: A7-333
                return opcodes::cmdOrnRegister(opCode, cpu);
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
                // see: A7-464
                return opcodes::cmdTeqRegister(opCode, cpu);
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
            // see: A7-374
            return opcodes::cmdRsbRegister(opCode, cpu);
        default:
            break;
    }

    UNDEFINED;
}

inline void coprocessorInstructions(uint32_t /*opCode*/, Cpu& /*cpu*/)
{
    // see: A5-156
    UNIMPLEMENTED;
}

inline void dataProcessingModifiedImmediate(uint32_t opCode, Cpu& cpu)
{
    const auto [Rd, Rn, op] = split<_<8, 4>, _<16, 4>, _<20, 6>>(opCode);

    // see: A5-136
    switch (op) {
        case 0b0000'0u ... 0b0000'1u:
            if (Rd != 0b1111u) {
                // see: A7-199
                return opcodes::cmdBitwiseImmediate<opcodes::Bitwise::AND>(opCode, cpu);
            }
            else {
                // see: A7-465
                return opcodes::cmdTstImmediate(opCode, cpu);
            }
        case 0b0001'0u ... 0b0001'1u:
            // see: A7-211
            return opcodes::cmdBitwiseImmediate<opcodes::Bitwise::BIC>(opCode, cpu);
        case 0b0010'0u ... 0b0010'1u:
            if (Rn != 0b1111u) {
                // see: A7-334
                return opcodes::cmdBitwiseImmediate<opcodes::Bitwise::ORR>(opCode, cpu);
            }
            else {
                // see: A7-312
                return opcodes::cmdMovImmediate<opcodes::Encoding::T2>(opCode, cpu);
            }
        case 0b0011'0u ... 0b0011'1u:
            if (Rn != 0b1111u) {
                // see: A7-332
                return opcodes::cmdOrnImmediate(opCode, cpu);
            }
            else {
                // see: A7-326
                return opcodes::cmdMvnImmediate(opCode, cpu);
            }
        case 0b0100'0u ... 0b0100'1u:
            if (Rd != 0b1111u) {
                // see: A7-238
                return opcodes::cmdBitwiseImmediate<opcodes::Bitwise::EOR>(opCode, cpu);
            }
            else {
                // see: A7-463
                return opcodes::cmdTeqImmediate(opCode, cpu);
            }
        case 0b1000'0u ... 0b1000'1u:
            if (Rd != 0b1111) {
                // see: A7-189
                return opcodes::cmdAddSubImmediate<opcodes::Encoding::T3, /*isSub*/ false>(opCode, cpu);
            }
            else {
                // see: A7-225
                return opcodes::cmdCmpImmediate<opcodes::Encoding::T1, /*isNegative*/ true>(opCode, cpu);
            }
        case 0b1010'0u ... 0b1010'1u:
            // see: A7-185
            return opcodes::cmdAdcSbcImmediate</*isSbc*/ false>(opCode, cpu);
        case 0b1011'0u ... 0b1011'1u:
            // see: A7-379
            return opcodes::cmdAdcSbcImmediate</*isSbc*/ true>(opCode, cpu);
        case 0b1101'0u ... 0b1101'1u:
            if (Rd != 0b1111u) {
                // see: A7-448
                return opcodes::cmdAddSubImmediate<opcodes::Encoding::T3, /*isSub*/ true>(opCode, cpu);
            }
            else {
                // see: A7-229
                return opcodes::cmdCmpImmediate<opcodes::Encoding::T2, /*isNegative*/ false>(opCode, cpu);
            }
        case 0b1110'0u ... 0b1110'1u:
            // see: A7-372
            return opcodes::cmdRsbImmediate<opcodes::Encoding::T2>(opCode, cpu);
        default:
            UNDEFINED;
    }
}

inline void dataProcessingPlainBinaryImmediate(uint32_t opCode, Cpu& cpu)
{
    const auto [Rn, op] = split<_<16, 4>, _<20, 5>>(opCode);

    // see: A5-139
    switch (op) {
        case 0b00000u:
            if (Rn != 0b1111u) {
                // see: A7-189
                return opcodes::cmdAddSubImmediate<opcodes::Encoding::T4, /*isSub*/ false>(opCode, cpu);
            }
            else {
                // see: A7-197
                return opcodes::cmdAdr<opcodes::Encoding::T3>(opCode, cpu);
            }
        case 0b00100u:
            // see: A7-312
            return opcodes::cmdMovImmediate<opcodes::Encoding::T3>(opCode, cpu);
        case 0b01010u:
            if (Rn != 0b1111u) {
                // see: A7-448
                return opcodes::cmdAddSubImmediate<opcodes::Encoding::T4, /*isSub*/ true>(opCode, cpu);
            }
            else {
                // see: A7-197
                return opcodes::cmdAdr<opcodes::Encoding::T2>(opCode, cpu);
            }
        case 0b01100u:
            // see: A7-317
            return opcodes::cmdMovt(opCode, cpu);
        case 0b10000u:
        case 0b10010u:
            // see: A7-415
            return opcodes::cmdSat</*isSigned*/ true>(opCode, cpu);
        case 0b10100u:
            // see: A7-382
            return opcodes::cmdBfx</*isSigned*/ true>(opCode, cpu);
        case 0b10110u:
            if (Rn != 0b1111u) {
                // see: A7-210
                return opcodes::cmdBfi(opCode, cpu);
            }
            else {
                // see: A7-209
                return opcodes::cmdBfc(opCode, cpu);
            }
        case 0b11000u:
        case 0b11010u:
            // see: A7-490
            return opcodes::cmdSat</*isSigned*/ false>(opCode, cpu);
        case 0b11100u:
            // see: A7-470
            return opcodes::cmdBfx</*isSigned*/ false>(opCode, cpu);
        default:
            break;
    }

    UNDEFINED;
}

inline void branchesAndMiscControl(uint32_t opCode, Cpu& cpu)
{
    const auto [op1, op] = split<_<12, 3>, _<20, 7>>(opCode);

    // see: A5-140
    switch (op1 & 0b101u) {
        case 0b000u:
            if ((op & 0b0111000u) != 0b0111000u) {
                // see: A7-207
                return opcodes::cmdBranch<opcodes::Encoding::T3>(opCode, cpu);
            }
            else {
                switch (op) {
                    case 0b011100'0u ... 0b011100'1u:
                        // see: A7-323
                        return opcodes::cmdMsr(opCode, cpu);
                    case 0b0111010u:
                        if (getPart<8, 3>(opCode) == 0b000u) {
                            switch (getPart<0, 8>(opCode)) {
                                case 0b00000000u:
                                    // see: A7-331
                                    return opcodes::cmdHint<opcodes::Hint::Nop>(opCode, cpu);
                                case 0b00000001u:
                                    // see: A7-562
                                    return opcodes::cmdHint<opcodes::Hint::Yield>(opCode, cpu);
                                case 0b00000010u:
                                    // see: A7-560
                                    return opcodes::cmdHint<opcodes::Hint::WaitForEvent>(opCode, cpu);
                                case 0b00000011u:
                                    // see: A7-561
                                    return opcodes::cmdHint<opcodes::Hint::WaitForInterrupt>(opCode, cpu);
                                case 0b00000100u:
                                    // see: A7-385
                                    return opcodes::cmdHint<opcodes::Hint::SendEvent>(opCode, cpu);
                                default:
                                    return;  // ignore
                            }
                        }
                        else {
                            UNDEFINED;
                        }
                    case 0b0111011u:
                        switch (getPart<4, 4>(opCode)) {
                            case 0b0010u:
                                // see: A7-223
                                return opcodes::cmdMiscControl<opcodes::Control::ClearExclusive>(opCode, cpu);
                            case 0b0100u:
                                // see: A7-237
                                return opcodes::cmdMiscControl<opcodes::Control::DataSynchronizationBarrier>(opCode, cpu);
                            case 0b0101u:
                                // see: A7-235
                                return opcodes::cmdMiscControl<opcodes::Control::DataMemoryBarrier>(opCode, cpu);
                            case 0b0110u:
                                // see: A7-241
                                return opcodes::cmdMiscControl<opcodes::Control::InstructionSynchronizationBarrier>(opCode, cpu);
                            default:
                                break;
                        }
                        break;
                    case 0b011111'0u ... 0b011111'1u:
                        // see: A7-322
                        return opcodes::cmdMrs(opCode, cpu);
                    case 0b1111111u:
                        // see: A7-471
                        return opcodes::cmdPermanentlyUndefined<opcodes::Encoding::T2>(opCode, cpu);
                    default:
                        break;
                }
            }
            break;
        case 0b001u:
            // see: A7-207
            return opcodes::cmdBranch<opcodes::Encoding::T4>(opCode, cpu);
        case 0b101u:
            // see: A7-216
            return opcodes::cmdBranchWithLinkImmediate(opCode, cpu);
        default:
            break;
    }

    UNDEFINED;
}

inline void storeSingleDataItem(uint32_t opCode, Cpu& cpu)
{
    const auto [op2, op1] = split<_<6, 6>, _<21, 3>>(opCode);

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
            UNDEFINED;
    }
}

inline void loadByteAndMemoryHints(uint32_t opCode, Cpu& cpu)
{
    const auto [op2, Rt, Rn, op1] = split<_<6, 6>, _<12, 4>, _<16, 4>, _<23, 2>>(opCode);

    // see: A5-146
    if (Rt != 0b1111u) {
        if (isBitClear<1>(op1)) {
            if (Rn == 0b1111u) {
                // see: A7-260
                return opcodes::cmdLoadLiteral<opcodes::Encoding::T1, uint8_t, /*isSignExtended*/ false>(opCode, cpu);
            }
            if (op1 == 0b01u || (op1 == 0b00u && ((op2 & 0b100100u) == 0b100100u || getPart<2, 4>(op2) == 0b1100u))) {
                if (isBitSet<0>(op1)) {
                    // see: A7-258
                    return opcodes::cmdLoadImmediate<opcodes::Encoding::T2, uint8_t, /*isSignExtended*/ false>(opCode, cpu);
                }
                else {
                    // see: A7-258
                    return opcodes::cmdLoadImmediate<opcodes::Encoding::T3, uint8_t, /*isSignExtended*/ false>(opCode, cpu);
                }
            }
            if (op1 == 0b00u && getPart<2, 4>(op2) == 0b1110u) {
                // see: A7-264
                return opcodes::cmdLoadRegisterUnprivileged<uint8_t, /*isSignExtended*/ false>(opCode, cpu);
            }
            if (op1 == 0b00u && op2 == 0b000000u) {
                // see: A7-262
                return opcodes::cmdLoadRegister<opcodes::Encoding::T2, uint8_t, /*isSignExtended*/ false>(opCode, cpu);
            }
        }
        else {
            if (Rn == 0b1111u) {
                // see: A7-284
                return opcodes::cmdLoadLiteral<opcodes::Encoding::T1, uint8_t, /*isSignExtended*/ true>(opCode, cpu);
            }
            if (op1 == 0b11 || (op1 == 0b10u && ((op2 & 0b100100u) == 0b100100u || getPart<2, 4>(op2) == 0b1100u))) {
                if (isBitSet<0>(op1)) {
                    // see: A7-282
                    return opcodes::cmdLoadImmediate<opcodes::Encoding::T1, uint8_t, /*isSignExtended*/ true>(opCode, cpu);
                }
                else {
                    // see: A7-282
                    return opcodes::cmdLoadImmediate<opcodes::Encoding::T2, uint8_t, /*isSignExtended*/ true>(opCode, cpu);
                }
            }
            if (op1 == 0b10u && getPart<2, 4>(op2) == 0b1110u) {
                // see: A7-288
                return opcodes::cmdLoadRegisterUnprivileged<uint8_t, /*isSignExtended*/ true>(opCode, cpu);
            }
            if (op1 == 0b10u && op2 == 0b000000u) {
                // see: A7-286
                return opcodes::cmdLoadRegister<opcodes::Encoding::T2, uint8_t, /*isSignExtended*/ true>(opCode, cpu);
            }
        }
    }
    else {
        if (isBitClear<1>(op1)) {
            if (Rn == 0b1111u) {
                // see: A7-341
                return opcodes::cmdPreloadDataLiteral(opCode, cpu);
            }
            if (op1 == 0b01u || (op1 == 0b00u && getPart<2, 4>(op2) == 0b1100u)) {
                // see: A7-340
                return opcodes::cmdPreloadDataImmediate(opCode, cpu);
            }
            if (op1 == 0b00u && op2 == 0b000000u) {
                // see: A7-342
                return opcodes::cmdPreloadDataRegister(opCode, cpu);
            }
        }
        else {
            if (Rn == 0b1111u || op1 == 0b11u || (op1 == 0b10u && getPart<2, 4>(op2) == 0b1100u)) {
                // see: A7-344
                return opcodes::cmdPreloadInstructionImmediate(opCode, cpu);
            }
            if (op1 == 0b10u && op2 == 0b000000u) {
                // see: A7-346
                return opcodes::cmdPreloadInstructionRegister(opCode, cpu);
            }
        }
    }

    UNDEFINED;
}

inline void loadHalfWordAndMemoryHints(uint32_t opCode, Cpu& cpu)
{
    const auto [op2, Rt, Rn, op1] = split<_<6, 6>, _<12, 4>, _<16, 4>, _<23, 2>>(opCode);

    // see: A5-145
    if (isBitSet<1>(op1)) {
        if (Rt != 0b1111u) {
            if (Rn == 0b1111u) {
                // see A7-276
                return opcodes::cmdLoadLiteral<opcodes::Encoding::T1, uint16_t, /*isSignExtended*/ false>(opCode, cpu);
            }
            if ((op1 == 0b00u && ((op2 & 0b100100u) == 0b100100u || getPart<2, 4>(op2) == 0b1100u)) || op1 == 0b01u) {
                if (isBitSet<0>(op1)) {
                    // see: A7-274
                    return opcodes::cmdLoadImmediate<opcodes::Encoding::T2, uint16_t, /*isSignExtended*/ false>(opCode, cpu);
                }
                else {
                    // see: A7-274
                    return opcodes::cmdLoadImmediate<opcodes::Encoding::T3, uint16_t, /*isSignExtended*/ false>(opCode, cpu);
                }
            }
            if (op1 == 0b00u && op2 == 0b000000u) {
                // see: A7-278
                return opcodes::cmdLoadRegister<opcodes::Encoding::T2, uint16_t, /*isSignExtended*/ false>(opCode, cpu);
            }
            if (op1 == 0b00u && getPart<2, 4>(op2) == 0b1110u) {
                // see: A7-280
                return opcodes::cmdLoadRegisterUnprivileged<uint16_t, /*isSignExtended*/ false>(opCode, cpu);
            }
        }
        else if ((op1 == 0b00u && (op2 == 0b000000u || getPart<2, 4>(op2) == 0b1100u)) || op1 == 0b01u) {
            return opcodes::cmdHint<opcodes::Hint::Nop>(opCode, cpu);  // software shouldn't use this encoding
        }
    }
    else {
        if (Rt != 0b1111u) {
            if (Rn == 0b1111u) {
                // see: A7-292
                return opcodes::cmdLoadLiteral<opcodes::Encoding::T1, uint16_t, /*isSignExtended*/ true>(opCode, cpu);
            }
            if ((op1 == 0b10u && ((op2 & 0b100100u) == 0b100100u || getPart<2, 4>(op2) == 0b1100u)) || op1 == 0b11u) {
                if (isBitSet<0>(op1)) {
                    // see: A7-290
                    return opcodes::cmdLoadImmediate<opcodes::Encoding::T1, uint16_t, /*isSignExtended*/ true>(opCode, cpu);
                }
                else {
                    // see: A7-290
                    return opcodes::cmdLoadImmediate<opcodes::Encoding::T2, uint16_t, /*isSignExtended*/ true>(opCode, cpu);
                }
            }
            if (op1 == 0b10u && op2 == 0b000000u) {
                // see: A7-294
                return opcodes::cmdLoadRegister<opcodes::Encoding::T2, uint16_t, /*isSignExtended*/ true>(opCode, cpu);
            }
            if (op1 == 0b10u && getPart<2, 4>(op2) == 0b1110u) {
                // see: A7-296
                return opcodes::cmdLoadRegisterUnprivileged<uint16_t, /*isSignExtended*/ true>(opCode, cpu);
            }
        }
        else {
            if ((op1 == 0b10u && (op2 == 0b000000u || getPart<2, 4>(op2) == 0b1100u)) || Rn == 0b1111u || op1 == 0b11u) {
                return opcodes::cmdHint<opcodes::Hint::Nop>(opCode, cpu);  // software shouldn't use this encoding
            }
        }
    }

    UNDEFINED;
}

inline void loadWord(uint32_t opCode, Cpu& cpu)
{
    const auto [op2, Rn, op1] = split<_<6, 6>, _<16, 4>, _<23, 2>>(opCode);

    // see: A5-144
    if (Rn != 0b1111u) {
        switch (op1) {
            case 0b01u:
                // see: A7-252
                return opcodes::cmdLoadImmediate<opcodes::Encoding::T3, uint32_t, /*isSignExtended*/ false>(opCode, cpu);
            case 0b00u:
                if ((op2 & 0b100100u) == 0b100100u || getPart<2, 4>(op2) == 0b1100u) {
                    // see: A7-252
                    return opcodes::cmdLoadImmediate<opcodes::Encoding::T4, uint32_t, /*isSignExtended*/ false>(opCode, cpu);
                }
                else if (getPart<2, 4>(op2) == 0b1110u) {
                    // see: A7-297
                    return opcodes::cmdLoadRegisterUnprivileged<uint32_t, /*isSignExtended*/ false>(opCode, cpu);
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
        if (isBitClear<1>(op1)) {
            // see: A7-254
            opcodes::cmdLoadLiteral<opcodes::Encoding::T2, uint32_t, /*isSignExtended*/ false>(opCode, cpu);
        }
    }

    UNDEFINED;
}

inline void dataProcessingRegister(uint32_t opCode, Cpu& cpu)
{
    const auto [op2, Rn, op1] = split<_<4, 4>, _<16, 4>, _<20, 4>>(opCode);

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
                    return opcodes::cmdReverseBytes<opcodes::Encoding::T2, uint32_t>(opCode, cpu);
                case 0b1001u:
                    // see: A7-364
                    return opcodes::cmdReverseBytes<opcodes::Encoding::T2, uint16_t>(opCode, cpu);
                case 0b1010u:
                    // see: A7-362
                    return opcodes::cmdReverseBits(opCode, cpu);
                case 0b1011u:
                    // see: A7-365
                    return opcodes::cmdReverseBytes<opcodes::Encoding::T2, uint16_t, /*isSignExtended*/ true>(opCode, cpu);
                default:
                    break;
            }
            break;
        case 0b1011u:
            if (op2 == 0b1000u) {
                // see: A7-224
                return opcodes::cmdClz(opCode, cpu);
            }
            break;
        default:
            break;
    }

    UNDEFINED;
}

inline void multiplicationAndAbsoluteDifference(uint32_t opCode, Cpu& cpu)
{
    const auto [op2, Ra, op1] = split<_<4, 2>, _<12, 4>, _<20, 3>>(opCode);

    // see: A5-154
    switch (op1) {
        case 0b000u:
            switch (op2) {
                case 0b00u:
                    if (Ra != 0b1111u) {
                        // see: A7-310
                        return opcodes::cmdMlaMls</*substract*/ false>(opCode, cpu);
                    }
                    else {
                        // see: A7-324
                        return opcodes::cmdMul<opcodes::Encoding::T2>(opCode, cpu);
                    }
                case 0b01u:
                    // see: A7-311
                    return opcodes::cmdMlaMls</*substract*/ true>(opCode, cpu);
                default:
                    break;
            }
    }

    UNDEFINED;
}

inline void longMultiplicationAndDivision(uint32_t opCode, Cpu& cpu)
{
    const auto [op2, op1] = split<_<4, 4>, _<20, 3>>(opCode);

    // see: A5-154
    switch (op1) {
        case 0b000u:
            if (op2 == 0b0000u) {
                // see: A7-412
                return opcodes::cmdMulLong</*isSigned*/ true>(opCode, cpu);
            }
            break;
        case 0b001u:
            if (op2 == 0b1111u) {
                // see: A7-383
                return opcodes::cmdDiv</*isSigned*/ true>(opCode, cpu);
            }
            break;
        case 0b010u:
            if (op2 == 0b0000u) {
                // see: A7-481
                return opcodes::cmdMulLong</*isSigned*/ false>(opCode, cpu);
            }
            break;
        case 0b011u:
            if (op2 == 0b1111u) {
                // see: A7-472
                return opcodes::cmdDiv</*isSigned*/ false>(opCode, cpu);
            }
            break;
        case 0b100u:
            if (op2 == 0b0000u) {
                // see: A7-396
                return opcodes::cmdMulAccumulateLong</*isSigned*/ true>(opCode, cpu);
            }
            break;
        case 0b110u:
            if (op2 == 0b0000u) {
                // see: A7-480
                return opcodes::cmdMulAccumulateLong</*isSigned*/ false>(opCode, cpu);
            }
            break;
        default:
            break;
    }

    UNDEFINED;
}

}  // namespace wo

void Cpu::step()
{
    auto& PC = m_registers.PC();
    m_currentInstructionAddress = PC;
    m_skipIncrementingPC = false;

    const auto opCodeHw1 = m_memory.read<uint16_t>(PC & ZEROS<1, uint32_t>);

    const auto op1 = getPart<11, 2>(opCodeHw1);

    if (op1 == 0u || (getPart<13, 3>(opCodeHw1) != 0b111u)) {
        switch (getPart<10, 6>(opCodeHw1)) {
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
            case 0b11100'0u ... 0b11100'1u:
                hw::handleUnconditionalBranch(opCodeHw1, *this);
                break;
            default:
                UNDEFINED;
        }

        if (!m_skipIncrementingPC) {
            PC += 2u;
        }
    }
    else {
        const auto opCodeHw2 = m_memory.read<uint16_t>((PC & ZEROS<1, uint32_t>)+2u);
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
                        UNDEFINED;
                }
                break;
            case 0b10u:
                if (isBitClear<15>(opCodeHw2)) {
                    if (isBitClear<5>(op2)) {
                        wo::dataProcessingModifiedImmediate(opCode, *this);
                    }
                    else {
                        wo::dataProcessingPlainBinaryImmediate(opCode, *this);
                    }
                }
                else {
                    wo::branchesAndMiscControl(opCode, *this);
                }
                break;
            case 0b11u:
                switch (op2) {
                    case 0b00'00000u ... 0b00'11111u:
                        if ((op2 & 0b1110001u) == 0u) {
                            wo::storeSingleDataItem(opCode, *this);
                        }
                        else {
                            switch (getPart<0, 3>(op2)) {
                                case 0b001u:
                                    wo::loadByteAndMemoryHints(opCode, *this);
                                    break;
                                case 0b011u:
                                    wo::loadHalfWordAndMemoryHints(opCode, *this);
                                    break;
                                case 0b101u:
                                    wo::loadWord(opCode, *this);
                                    break;
                                default:
                                    UNDEFINED;
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
                        UNDEFINED;
                }
                break;
            default:
                UNDEFINED;
        }

        if (!m_skipIncrementingPC) {
            PC += 4u;
        }
    }

    if (isInItBlock() && !m_skipAdvancingIT) {
        advanceCondition();
    }
    m_skipAdvancingIT = false;
}  // namespace wo

}  // namespace stm32
