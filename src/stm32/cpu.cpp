// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "cpu.hpp"

#include <cassert>

namespace {
inline bool is32bitInstruction(uint8_t firstByte)
{
    const auto maskedOpCode = firstByte & 0b11111'000u;
    return maskedOpCode == 0b11101'000u || maskedOpCode == 0b11110'000u || maskedOpCode == 0b11111'000u;
}

}  // namespace

namespace stm32 {
void VirtualCpu::reset()
{
    m_registers.reset();
}

inline void handleMathInstruction(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
    /// see A5.2.1
    switch (math::maskedShift<9, 5>(opCode)) {
        case 0b000'00u ... 0b000'11u:
            switch ((opCode >> 6u) & 0b11111u) {
                case 0b00000u:
                    // TODO: A7-312
                    return;
                default:
                    // see: A7-298
                    return opcodes::cmdShiftImmediate<opcodes::Encoding::T1, math::ShiftType::LSL>(opCode, registers, memory);
            }
        case 0b001'00u ... 0b001'11u:
            // see: A7-302
            return opcodes::cmdShiftImmediate<opcodes::Encoding::T1, math::ShiftType::LSR>(opCode, registers, memory);
        case 0b010'00u ... 0b010'11u:
            // see: A7-203
            return opcodes::cmdShiftImmediate<opcodes::Encoding::T1, math::ShiftType::ASR>(opCode, registers, memory);
        case 0b01100u:
            // TODO: A7-191
            return;
        case 0b01101u:
            // TODO: A7-450
            return;
        case 0b01110u:
            // see: A7-189
            return opcodes::cmdAddSubImmediate<opcodes::Encoding::T1, false>(opCode, registers, memory);
        case 0b01111u:
            // TODO: A7-448
            return;
        case 0b100'00u ... 0b100'11u:
            // TODO: A7-312
            return;
        case 0b101'00u ... 0b101'11u:
            // TODO: A7-229
            return;
        case 0b110'00u ... 0b110'11u:
            // TODO: A7-189
            return;
        case 0b111'00u ... 0b111'11u:
            // TODO: A7-448
            return;
        default:
            break;
    }

    assert(false);  // UNPREDICTABLE
}

inline void handleDataProcessingInstruction(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
    // see A5.2.2
    switch ((opCode >> 6u) & 0b1111u) {
        case 0b0000u:
            // TODO: A7-201
            return;
        case 0b0001u:
            // TODO: A7-239
            return;
        case 0b0010u:
            // TODO: A7-300
            return;
        case 0b0011u:
            // TODO: A7-304
            return;
        case 0b0100u:
            // TODO: A7-205
            return;
        case 0b0101u:
            // TODO: A7-187
            return;
        case 0b0110u:
            // TODO: A7-380
            return;
        case 0b0111u:
            // TODO: A7-368
            return;
        case 0b1000u:
            // TODO: A7-466
            return;
        case 0b1001u:
            // TODO: A7-372
            return;
        case 0b1010u:
            // TODO: A7-231
            return;
        case 0b1011u:
            // TODO: A7-227
            return;
        case 0b1100u:
            // TODO: A7-336
            return;
        case 0b1101u:
            // TODO: A7-234
            return;
        case 0b1110u:
            // TODO: A7-213
            return;
        case 0b1111u:
            // TODO: A7-238
            return;
        default:
            break;
    }

    assert("UNPREDICTABLE");
}

inline void handleSpecialDataInstruction(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
    // see A5.2.3
    switch ((opCode >> 6u) & 0b1111u) {
        case 0b00'00u ... 0b00'11u:
            // TODO: A7-191
            return;
        case 0b0101u:
        case 0b011'0u ... 0b011'1u:
            // TODO: A7-231
            return;
        case 0b10'00u ... 0b10'11u:
            // TODO: A7-314
            return;
        case 0b110'0u ... 0b110'1u:
            // TODO: A7-218
            return;
        case 0b111'0u ... 0b111'1u:
            // TODO: A7-217
            return;
        default:
            break;
    }

    assert("UNPREDICTABLE");
}

inline void handleLoadFromLiteralPool(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
    // TODO: A7.7.43
}

inline void handleLoadStoreSingleDataItem(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
    // see A5.2.4
    switch (opCode >> 12u) {
        case 0b0101u:
            switch ((opCode >> 9u) & 0b111u) {
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
                    break;
            }
            break;
        case 0b0110u:
            switch ((opCode >> 9u) & 0b111u) {
                case 0b0'00u ... 0b0'11u:
                    // TODO: A7-426
                    return;
                case 0b1'00u ... 0b1'11u:
                    // TODO: A7-252
                    return;
                default:
                    break;
            }
            break;
        case 0b0111u:
            switch ((opCode >> 9u) & 0b111u) {
                case 0b0'00u ... 0b0'11u:
                    // TODO: A7-430
                    return;
                case 0b1'00u ... 0b1'11u:
                    // TODO: A7-258
                    return;
                default:
                    break;
            }
            break;
        case 0b1000u:
            switch ((opCode >> 9u) & 0b111u) {
                case 0b0'00u ... 0b0'11u:
                    // TODO: A7-442
                    return;
                case 0b1'00u ... 0b1'11u:
                    // TODO: A7-274
                    return;
                default:
                    break;
            }
            break;
        case 0b1001u:
            switch ((opCode >> 9u) & 0b111u) {
                case 0b0'00u ... 0b0'11u:
                    // TODO: A7-426
                    return;
                case 0b1'00u ... 0b1'11u:
                    // TODO: A7-252
                    return;
                default:
                    break;
            }
            break;
    }

    assert("UNPREDICTABLE");
}

inline void handleGeneratePcRelativeAddress(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
    // TODO: A7.7.7
}

inline void handleGenerateSpRelativeAddress(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
    // TODO: A7.7.5
}

inline void handleMiscInstruction(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
    // see A5.2.5
    switch (static_cast<uint16_t>(opCode >> 5u) & 0b1111111u) {
        case 0b00000'00u ... 0b00000'11u:
            // TODO: A7-193
            return;
        case 0b00001'00u ... 0b00001'11u:
            // TODO: A7-452
            return;
        case 0b0001'000u ... 0b0001'111u:
            // TODO: A7-219
            return;
        case 0b001000'0u ... 0b001000'1u:
            // TODO: A7-461
            return;
        case 0b001001'0u ... 0b001001'1u:
            // TODO: A7-459
            return;
        case 0b001010'0u ... 0b001010'1u:
            // TODO: A7-500
            return;
        case 0b001011'0u ... 0b001011'1u:
            // TODO: A7-498
            return;
        case 0b0011'000u ... 0b0011'111u:
            // TODO: A7-219
            return;
        case 0b0110011u:
            // TODO: B5-731
            return;
        case 0b1001'000u ... 0b1001'111u:
            // TODO: A7-219
            return;
        case 0b101000'0u ... 0b101000'1u:
            // TODO: A7-363
            return;
        case 0b101001'0u ... 0b101001'1u:
            // TODO: A7-364
            return;
        case 0b101011'0u ... 0b101011'1u:
            // TODO: A7-365
            return;
        case 0b1011'000u ... 0b1011'111u:
            // TODO A7-219
            return;
        case 0b110'0000u ... 0b110'1111u:
            // TODO: A7-348
            return;
        case 0b1110'000u ... 0b1110'111u:
            // TODO: A7-215
            return;
        case 0b1111'000u ... 0b1111'111u:
            // see A5-133
            switch (opCode & 0b1111u) {
                case 0b0000u:
                    switch (static_cast<uint16_t>(opCode >> 4u) & 0b1111u) {
                        case 0b0001u:
                            // TODO: A7-562
                            return;
                        case 0b0010u:
                            // TODO: A7-560
                            return;
                        case 0b0011u:
                            // TODO: A7-561
                            return;
                        case 0b0100u:
                            // TODO: A7-385
                            return;
                        default:
                            break;
                    }
                    break;
                default:
                    // TODO: A7-242
                    return;
            }
            return;
        default:
            break;
    }

    assert("UNPREDICTABLE");
}

inline void handleStoreMultipleRegisters(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
    // TODO: A7.7.156
}

inline void handleLoadMultipleRegisters(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
    // TODO: A7.7.40
}

inline void handleConditionalBranch(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
    // see A5.2.6
    switch ((opCode >> 8u) & 0b1111u) {
        case 0b1110u:
            // TODO: A7-471
            return;
        case 0b1111u:
            // TODO: A7-455
            return;
        default:
            // TODO: A7-207
            return;
    }
}

inline void handleUnconditionalBranch(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
    // TODO: A7.7.12
}

void VirtualCpu::step()
{
    auto& PC = m_registers.reg(RegisterType::PC);

    const auto opCodeHigh = m_memory.read(PC);
    if (is32bitInstruction(opCodeHigh)) {
        // TODO: handle 32 bit instructions
    }
    else {
        const uint16_t opCode = static_cast<uint16_t>(static_cast<uint16_t>(opCodeHigh) << 8u) | m_registers.reg(RegisterType::PC);

        switch (opCodeHigh >> 2u) {
            case 0b00'0000u ... 0b00'1111u:
                handleMathInstruction(opCode, m_registers, m_memory);
                break;
            case 0b010000u:
                handleDataProcessingInstruction(opCode, m_registers, m_memory);
                break;
            case 0b010001u:
                handleSpecialDataInstruction(opCode, m_registers, m_memory);
                break;
            case 0b01001'0u ... 0b01001'1u:
                handleLoadFromLiteralPool(opCode, m_registers, m_memory);
                break;
            case 0b0101'00u ... 0b0101'11u:
            case 0b011'000u ... 0b011'111u:
            case 0b100'000u ... 0b100'111u:
                handleLoadStoreSingleDataItem(opCode, m_registers, m_memory);
                break;
            case 0b10100'0u ... 0b10100'1u:
                handleGeneratePcRelativeAddress(opCode, m_registers, m_memory);
                break;
            case 0b10101'0u ... 0b10101'1u:
                handleGenerateSpRelativeAddress(opCode, m_registers, m_memory);
                break;
            case 0b1011'00u ... 0b1011'11u:
                handleMiscInstruction(opCode, m_registers, m_memory);
                break;
            case 0b11000'0u ... 0b11000'1u:
                handleStoreMultipleRegisters(opCode, m_registers, m_memory);
                break;
            case 0b11001'0u ... 0b11001'1u:
                handleLoadMultipleRegisters(opCode, m_registers, m_memory);
                break;
            case 0b1101'00u ... 0b1101'11u:
                handleConditionalBranch(opCode, m_registers, m_memory);
                break;
            case 0b11100'0u ... 0b11100'0u:
                handleUnconditionalBranch(opCode, m_registers, m_memory);
                break;
            default:
                assert("UNPREDICTABLE");
                break;
        }
    }
}


}  // namespace stm32