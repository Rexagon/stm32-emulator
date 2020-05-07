// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "cpu.hpp"

#include <cassert>

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
Cpu::Cpu(const Memory::Config& memoryConfig)
    : m_registers{}
    , m_systemRegisters{}
    , m_sysTickRegisters{}
    , m_nvicRegisters{}
    , m_memory{memoryConfig}
    , m_mpu{*this}
    , m_currentMode{}
    , m_exceptionActive{}
{
}

void Cpu::reset()
{
    // see: B1.5.5
    m_registers.reset();

    m_exceptionActive.reset();

    m_systemRegisters.reset();
    m_sysTickRegisters.reset();
    m_nvicRegisters.reset();
    m_mpu.reset();
    // TODO: clear exclusive local
    clearEventRegister();

    // const auto vectorTable = math::combine<uint32_t>(math::Part<0, 7>{0u}, math::Part<7, 25, uint32_t>{m_systemRegisters.VTOR().TBLOFF});

    // m_registers.SP_main() = MemA_with_priv[vectortable, 4, AccType_VECTABLE] AND 0xFFFFFFFC<31:0>;
    m_registers.SP_process() &= utils::ZEROS<2, uint32_t>;  // ((bits(30) UNKNOWN):'00');
    m_registers.LR() = std::numeric_limits<uint32_t>::max();

    // tmp = MemA_with_priv[vectortable+4, 4, AccType_VECTABLE];
    // tbit = tmp<0>;

    // NOTE: m_registers.APSR() is unknown
    m_registers.IPSR().exceptionNumber = 0u;
    m_registers.EPSR().T = false;  // TODO: change to tbit
    m_registers.EPSR().ITlo = 0u;
    m_registers.EPSR().IThi = 0u;
    branchWritePC(0u);  // TODO: change to tmp & ~1u
}

void Cpu::branchWritePC(uint32_t address)
{
    m_registers.PC() = address & ~uint32_t{0b1u};
}

void Cpu::bxWritePC(uint32_t address)
{
    if (m_currentMode == ExecutionMode::Handler && utils::getPart<28, 4>(address) == 0b1111u) {
        // TODO: ExceptionReturn(address<27:0>); // see B1-597
    }
    else {
        m_registers.EPSR().T = address & utils::RIGHT_BIT<uint32_t>;
        m_registers.PC() = address & ~utils::RIGHT_BIT<uint32_t>;
    }
}

void Cpu::blxWritePC(uint32_t address)
{
    m_registers.EPSR().T = address & utils::RIGHT_BIT<uint32_t>;
    // TODO: if EPSR.T == 0, a UsageFault(‘Invalid State’) is taken on the next instruction
    m_registers.PC() = address & ~utils::RIGHT_BIT<uint32_t>;
}

auto Cpu::currentCondition() const -> uint8_t
{
    const auto ITSTATE = m_registers.ITSTATE();

    if (ITSTATE & 0x0fu) {
        return static_cast<uint8_t>(ITSTATE >> 4u);
    }
    else if (ITSTATE == 0x00u) {
        return 0b1110u;
    }
    UNPREDICTABLE;
}

auto Cpu::conditionPassed() const -> bool
{
    const auto condition = currentCondition() & 0x0fu;
    const auto APSR = m_registers.APSR();

    bool result;
    switch (condition >> 1u) {
        case 0b000u:
            result = APSR.Z;
            break;
        case 0b001u:
            result = APSR.C;
            break;
        case 0b010u:
            result = APSR.N;
            break;
        case 0b011u:
            result = APSR.V;
            break;
        case 0b100u:
            result = APSR.C && !APSR.Z;
            break;
        case 0b101u:
            result = APSR.N == APSR.V;
            break;
        case 0b110u:
            result = APSR.N == APSR.V && !APSR.Z;
            break;
        case 0b111u:
            result = true;
            break;
        default:
            UNPREDICTABLE;
    }

    if ((condition & 0b1u) && (condition != 0x0fu)) {
        result = !result;
    }

    return result;
}

auto Cpu::isInItBlock() const -> bool
{
    return m_registers.ITSTATE() & utils::ONES<4, uint8_t>;
}

auto Cpu::isLastInItBlock() const -> bool
{
    return (m_registers.ITSTATE() & utils::ONES<4, uint8_t>) == 0b1000u;
}

void Cpu::advanceCondition()
{
    using namespace utils;

    auto& EPSR = m_registers.EPSR();
    auto ITSTATE = m_registers.ITSTATE();

    if (ITSTATE & 0b111u) {
        ITSTATE = static_cast<uint8_t>((ITSTATE & ZEROS<5, uint8_t>) | ((ITSTATE << 1u) & ONES<5, uint8_t>));
        EPSR.ITlo = getPart<2, 6>(ITSTATE) & ONES<6, uint8_t>;
        EPSR.IThi = getPart<0, 2>(ITSTATE) & ONES<2, uint8_t>;
    }
    else {
        EPSR.ITlo = 0u;
        EPSR.IThi = 0u;
    }
}

auto Cpu::isInPrivilegedMode() const -> bool
{
    return m_currentMode == ExecutionMode::Handler || !m_registers.CONTROL().nPRIV;
}

auto Cpu::executionPriority() const -> int32_t
{
    auto highestPRI = 256;  // Priority of Thread mode with no active exceptions
    // The value is PriorityMax + 1 = 256
    // (configurable priority maximum bit field is 8 bits)

    auto boostedPRI = 256;  // Priority influence of BASEPRI, PRIMASK and FAULTMASK

    auto subGroupShift = m_systemRegisters.AIRCR().PRIGROUP;
    auto groupValue = utils::lsl(std::uint32_t{0b10}, subGroupShift);

    for (size_t i = 2; i < 512; ++i) {
        // TODO:
        // if ExceptionActive[i] == '1' then
        //      if ExceptionPriority[i] < highestpri then
        //          highestpri = ExceptionPriority[i];
        //          // Include the PRIGROUP effect
        //          subgroupvalue = highestpri MOD groupvalue;
        //          highestpri = highestpri - subgroupvalue;
    }

    if (m_registers.BASEPRI().level != 0) {
        boostedPRI = m_registers.BASEPRI().level;

        const int32_t subGroupValue = boostedPRI % static_cast<int32_t>(groupValue);
        boostedPRI -= subGroupValue;
    }

    if (m_registers.PRIMASK().PM) {
        boostedPRI = 0;
    }

    if (m_registers.FAULTMASK().FM) {
        boostedPRI = -1;
    }

    return std::min(boostedPRI, highestPRI);
}

void Cpu::pushStack(ExceptionType exceptionType)
{
    using namespace utils;

    const auto frameSize = 0x20u;
    const auto forceAlign = m_systemRegisters.CCR().STKALIGN;

    const auto spMask = ~utils::combine<uint32_t>(utils::Part<0, 2>{0u}, utils::Part<3, 1>{forceAlign});

    bool framePtrAlign{};
    uint32_t framePtr{};
    if (m_registers.CONTROL().SPSEL && m_currentMode == ExecutionMode::Thread) {
        auto& SP_process = m_registers.SP_process();

        framePtrAlign = utils::getPart<2, 1>(SP_process) && forceAlign;
        SP_process = (SP_process - frameSize) & spMask;
        framePtr = SP_process;
    }
    else {
        auto& SP_main = m_registers.SP_main();

        framePtrAlign = utils::getPart<2, 1>(SP_main) && forceAlign;
        SP_main = (SP_main - frameSize) & spMask;
        framePtr = SP_main;
    }

    const auto [xPSRlo, xPSRhi] = split<uint32_t, Part<0, 9, uint32_t>, Part<10, 22, uint32_t>>(m_registers.xPSR());

    m_mpu.alignedMemoryWrite(framePtr + 0x0u, R(0));
    m_mpu.alignedMemoryWrite(framePtr + 0x4u, R(1));
    m_mpu.alignedMemoryWrite(framePtr + 0x8u, R(2));
    m_mpu.alignedMemoryWrite(framePtr + 0xCu, R(3));
    m_mpu.alignedMemoryWrite(framePtr + 0x10u, R(12));
    m_mpu.alignedMemoryWrite(framePtr + 0x14u, m_registers.LR());
    m_mpu.alignedMemoryWrite(framePtr + 0x18u, returnAddress(exceptionType));
    m_mpu.alignedMemoryWrite(framePtr + 0x1Cu,
                             combine<uint32_t>(Part<0, 9, uint32_t>{xPSRlo}, Part<9, 1>{framePtrAlign}, Part<10, 22, uint32_t>{xPSRhi}));

    if (m_currentMode == ExecutionMode::Handler) {
        m_registers.LR() = combine<uint32_t>(Part<0, 4>{0b0001u}, Part<4, 28, uint32_t>{ONES<28, uint32_t>});
    }
    else {
        m_registers.LR() =
            combine<uint32_t>(Part<0, 2>{0b01u}, Part<2, 1>{m_registers.CONTROL().SPSEL}, Part<3, 29, uint32_t>{ONES<29, uint32_t>});
    }
}

void Cpu::exceptionTaken(ExceptionType exceptionType)
{
    using namespace utils;

    // R[0] - R[3], R[12] are UNKNOWN
    const auto vectorTablePtr = combine<uint32_t>(Part<0, 7>{0u}, Part<7, 25, uint32_t>{m_systemRegisters.VTOR().TBLOFF});
    const auto exceptionHandlerPtr = m_mpu.alignedMemoryRead<uint32_t>(vectorTablePtr + exceptionType * 4u);
    branchWritePC(exceptionHandlerPtr);

    m_currentMode = ExecutionMode::Handler;
    // APSR is UNKNOWN
    m_registers.IPSR().exceptionNumber = getPart<0, 9>(static_cast<uint16_t>(exceptionType)) & ONES<9, uint16_t>;
    // m_registers.EPSR().T =
}

auto Cpu::returnAddress(ExceptionType /*exceptionType*/) -> uint32_t
{
    // TODO: B1-590
    return uint32_t{};
}

inline void handleMathInstruction(uint16_t opCode, Cpu& cpu)
{
    /// see A5.2.1
    switch (utils::getPart<9, 5>(opCode)) {
        case 0b000'00u ... 0b000'11u:
            switch (utils::getPart<6, 5>(opCode)) {
                case 0b00000u:
                    // see: A7-312
                    return opcodes::cmdMovRegister<opcodes::Encoding::T2>(opCode, cpu);
                default:
                    // see: A7-298
                    return opcodes::cmdShiftImmediate<opcodes::Encoding::T1, utils::ShiftType::LSL>(opCode, cpu);
            }
        case 0b001'00u ... 0b001'11u:
            // see: A7-302
            return opcodes::cmdShiftImmediate<opcodes::Encoding::T1, utils::ShiftType::LSR>(opCode, cpu);
        case 0b010'00u ... 0b010'11u:
            // see: A7-203
            return opcodes::cmdShiftImmediate<opcodes::Encoding::T1, utils::ShiftType::ASR>(opCode, cpu);
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
    switch (utils::getPart<6, 4>(opCode)) {
        case 0b0000u:
            // see: A7-201
            return opcodes::cmdBitwiseRegister<opcodes::Encoding::T1, opcodes::Bitwise::AND>(opCode, cpu);
        case 0b0001u:
            // see: A7-239
            return opcodes::cmdBitwiseRegister<opcodes::Encoding::T1, opcodes::Bitwise::EOR>(opCode, cpu);
        case 0b0010u:
            // see: A7-300
            return opcodes::cmdShiftRegister<opcodes::Encoding::T1, utils::ShiftType::LSL>(opCode, cpu);
        case 0b0011u:
            // see: A7-304
            return opcodes::cmdShiftRegister<opcodes::Encoding::T1, utils::ShiftType::LSR>(opCode, cpu);
        case 0b0100u:
            // see: A7-205
            return opcodes::cmdShiftRegister<opcodes::Encoding::T1, utils::ShiftType::ASR>(opCode, cpu);
        case 0b0101u:
            // see: A7-187
            return opcodes::cmdAdcSbcRegister<opcodes::Encoding::T1, /* isSbc */ false>(opCode, cpu);
        case 0b0110u:
            // see: A7-380
            return opcodes::cmdAdcSbcRegister<opcodes::Encoding::T1, /* isSbc */ true>(opCode, cpu);
        case 0b0111u:
            // see: A7-368
            return opcodes::cmdShiftRegister<opcodes::Encoding::T1, utils::ShiftType::ROR>(opCode, cpu);
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
    switch (utils::getPart<6, 4>(opCode)) {
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

inline void handleLoadFromLiteralPool(uint16_t /*opCode*/, Cpu& /*cpu*/)
{
    // TODO: A7.7.43
}

inline void handleLoadStoreSingleDataItem(uint16_t opCode, Cpu& /*cpu*/)
{
    // see A5.2.4
    switch (utils::getPart<12, 4>(opCode)) {
        case 0b0101u:
            switch (utils::getPart<9, 3>(opCode)) {
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
            switch (utils::getPart<9, 3>(opCode)) {
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
            switch (utils::getPart<9, 3>(opCode)) {
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
            switch (utils::getPart<9, 3>(opCode)) {
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
            switch (utils::getPart<9, 3>(opCode)) {
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
    switch (utils::getPart<5, 7>(opCode)) {
        case 0b00000'00u ... 0b00000'11u:
            // see: A7-193
            return opcodes::cmdAddSubSpPlusImmediate<opcodes::Encoding::T2, /*isSub*/ false>(opCode, cpu);
        case 0b00001'00u ... 0b00001'11u:
            // see: A7-452
            return opcodes::cmdAddSubSpPlusImmediate<opcodes::Encoding::T1, /*isSub*/ true>(opCode, cpu);
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
            // see: B5-731
            return opcodes::cmdCps(opCode, cpu);
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
            switch (utils::getPart<0, 4>(opCode)) {
                case 0b0000u:
                    switch (utils::getPart<4, 4>(opCode)) {
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
                            return;  // ignore other
                    }
                default:
                    // TODO: A7-242
                    return;
            }
        default:
            UNPREDICTABLE;
    }
}

inline void handleStoreMultipleRegisters(uint16_t /*opCode*/, Cpu& /*cpu*/)
{
    // TODO: A7.7.156
}

inline void handleLoadMultipleRegisters(uint16_t /*opCode*/, Cpu& /*cpu*/)
{
    // TODO: A7.7.40
}

inline void handleConditionalBranch(uint16_t opCode, Cpu& cpu)
{
    // see A5.2.6
    switch (utils::getPart<8, 4>(opCode)) {
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

    const auto opCodeHw1 = m_memory.read<uint16_t>(PC & ~utils::RIGHT_BIT<uint32_t>);
    PC += 2u;

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
        switch (utils::getPart<2, 6>(opCodeHw1)) {
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
    }
}

}  // namespace stm32
