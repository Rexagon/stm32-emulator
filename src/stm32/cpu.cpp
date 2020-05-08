// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "cpu.hpp"

namespace stm32
{
using namespace utils;

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
    m_registers.SP_process() &= ZEROS<2, uint32_t>;  // ((bits(30) UNKNOWN):'00');
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

void Cpu::branchWritePC(uint32_t address, bool skipIncrementingPC)
{
    m_registers.PC() = address & ZEROS<1, uint32_t>;
    m_skipIncrementingPC = skipIncrementingPC;
}

void Cpu::bxWritePC(uint32_t address, bool skipIncrementingPC)
{
    if (m_currentMode == ExecutionMode::Handler && getPart<28, 4>(address) == 0b1111u) {
        // TODO: ExceptionReturn(address<27:0>); // see B1-597
    }
    else {
        m_registers.EPSR().T = address & RIGHT_BIT<uint32_t>;
        m_registers.PC() = address & ZEROS<1, uint32_t>;
        m_skipIncrementingPC = skipIncrementingPC;
    }
}

void Cpu::blxWritePC(uint32_t address, bool skipIncrementingPC)
{
    m_registers.EPSR().T = address & utils::RIGHT_BIT<uint32_t>;
    // TODO: if EPSR.T == 0, a UsageFault(‘Invalid State’) is taken on the next instruction
    m_registers.PC() = address & ZEROS<1, uint32_t>;
    m_skipIncrementingPC = skipIncrementingPC;
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

    auto ITSTATE = m_registers.ITSTATE();

    if (ITSTATE & 0b111u) {
        ITSTATE = static_cast<uint8_t>((ITSTATE & ZEROS<5, uint8_t>) | ((ITSTATE << 1u) & ONES<5, uint8_t>));
        m_registers.setITSTATE(ITSTATE);
    }
    else {
        m_registers.setITSTATE(0u);
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

void Cpu::exceptionEntry(ExceptionType exceptionType)
{
    // TODO: see DerivedLateArrival

    pushStack(exceptionType);
    exceptionTaken(exceptionType);
}

void Cpu::pushStack(ExceptionType exceptionType)
{
    using namespace utils;

    const auto frameSize = 0x20u;
    const auto forceAlign = m_systemRegisters.CCR().STKALIGN;

    const auto spMask = ~utils::combine<uint32_t>(utils::Part<0, 2>{0u}, utils::Part<3, 1>{forceAlign});

    bool framePtrAlign;
    uint32_t framePtr;
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

    m_registers.IPSR().exceptionNumber = getPart<0, 9>(static_cast<uint16_t>(exceptionType)) & ONES<9, uint16_t>;

    m_registers.EPSR().T = exceptionHandlerPtr & RIGHT_BIT<uint32_t>;
    m_registers.EPSR().ITlo = 0u;
    m_registers.EPSR().IThi = 0u;

    m_registers.CONTROL().SPSEL = false;  // current stack is Main

    m_exceptionActive.set(exceptionType, true);
    // TODO: update system registers as appropriate. See B1.5.14
    setEventRegister();
    instructionSynchronizationBarrier(0b1111u);
}

auto Cpu::returnAddress(ExceptionType exceptionType) -> uint32_t
{
    switch (exceptionType) {
        case ExceptionType::NMI:
            return nextInstructionAddress();

        case HardFault:
            return currentInstructionAddress();  // or next if async

        case MemManage:
            return currentInstructionAddress();

        case BusFault:
            return currentInstructionAddress();  // or next if async

        case UsageFault:
            return currentInstructionAddress();

        case SVCall:
            return nextInstructionAddress();

        case PendSV:
            return nextInstructionAddress();

        case SysTick:
            return nextInstructionAddress();

        default:
            if (exceptionType > 16u) {
                return nextInstructionAddress();
            }
            else {
                UNPREDICTABLE;
            }
    }
}

auto Cpu::nextInstructionAddress() const -> uint32_t
{
    if (m_skipIncrementingPC) {
        return m_registers.PC();
    }

    return m_nextInstructionAddress;
}

void Cpu::instructionSynchronizationBarrier(uint8_t /*option*/)
{
    // TODO: implement
    // Instruction Synchronization Barrier flushes the pipeline in the processor, so that all instructions following the ISB
    // are fetched from cache or memory after the instruction has completed. It ensures that the effects of context altering
    // operations, such as those resulting from read or write accesses to the system control space (SCS), that completed
    // before the ISB instruction are visible to the instructions fetched after the ISB.
}

}  // namespace stm32
