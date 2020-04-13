#pragma once

#include <gtest/gtest.h>

#include <iostream>
#include <stm32/system_control_registers.hpp>
#include <typeinfo>

namespace details
{
enum RegisterSize
{
    Byte = 1u,
    HalfWord = 2u,
    Word = 4u
};
}  // namespace details

#define CHECK_REGISTER(T, size)                        \
    std::cout << #T << ": " << sizeof(T) << std::endl; \
    ASSERT_EQ(sizeof(T), size)

TEST(system_control_registers, alignment)
{
    using namespace details;

    CHECK_REGISTER(CpuIdBaseRegister, Word);
    CHECK_REGISTER(InterruptControlAndStateRegister, Word);
    CHECK_REGISTER(VectorTableOffsetRegister, Word);
    CHECK_REGISTER(ApplicationInterruptAndResetControlRegister, HalfWord);
    CHECK_REGISTER(SystemControlRegister, Word);
    CHECK_REGISTER(ConfigurationAndControlRegister, Word);
    CHECK_REGISTER(SystemHandlerPriorityRegister, Word);
    CHECK_REGISTER(SystemHandlerControlAndStateRegister, Word);

    CHECK_REGISTER(MemManageStatusRegister, Byte);
    CHECK_REGISTER(BusFaultStatusRegister, Byte);
    CHECK_REGISTER(UsageFaultStatusRegister, HalfWord);
    CHECK_REGISTER(ConfigurableFaultStatusRegister, Word);

    CHECK_REGISTER(HardFaultStatusRegister, Word);
    CHECK_REGISTER(MemManageFaultAddressRegister, Word);
    CHECK_REGISTER(BusFaultAddressRegister, Word);
    CHECK_REGISTER(AuxiliaryFaultStatusRegister, Word);
    CHECK_REGISTER(CoprocessorAccessControlRegister, Word);
    CHECK_REGISTER(InterruptControllerTypeRegister, Word);
    CHECK_REGISTER(AuxiliaryControlRegister, Word);
    CHECK_REGISTER(SoftwareTriggeredInterruptRegister, Word);

    CHECK_REGISTER(SysTickControlAndStatusRegister, Word);
    CHECK_REGISTER(SysTickReloadValueRegister, Word);
    CHECK_REGISTER(SysTickCurrentValueRegister, Word);
    CHECK_REGISTER(SysTickCalibrationValueRegister, Word);
}
