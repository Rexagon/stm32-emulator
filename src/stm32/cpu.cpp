// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "cpu.hpp"

#include <cassert>

namespace
{
inline bool is32bitInstruction(uint8_t firstByte)
{
    const auto maskedOpCode = firstByte & 0b11111000u;
    return maskedOpCode == 0b11101000u || maskedOpCode == 0b11110000u || maskedOpCode == 0b11111000u;
}

}  // namespace

namespace stm32
{
void VirtualCpu::reset()
{
    m_registers.reset();
}

inline void handleMathInstruction(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
    // TODO: A5.2.1
}

inline void handleDataProcessingInstruction(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
    // TODO: A5.2.2
}

inline void handleSpecialDataInstruction(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
    // TODO: A5.2.3
}

inline void handleLoadFromLiteralPool(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
    // TODO: A7.7.43
}

inline void handleLoadStoreSingleDataItem(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
    // TODO: A5.2.4
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
    // TODO: A5.2.5
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
    // TODO: A5.2.6
}

inline void handleUnconditionalBranch(uint16_t opCode, CpuRegisterSet& registers, Memory& memory)
{
    // TODO: A7.7.12
}

void VirtualCpu::step()
{
    auto& PC = m_registers.reg(RegisterType::PC);

    const auto opCodeHigh = m_memory.read(PC);
    if (is32bitInstruction(opCodeHigh))
    {
        // TODO: handle 32 bit instructions
    }
    else
    {
        const uint16_t opCode =
            static_cast<uint16_t>(static_cast<uint16_t>(opCodeHigh) << 8u) | m_registers.reg(RegisterType::PC);

        switch (opCodeHigh >> 2u)
        {
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
