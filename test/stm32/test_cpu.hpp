#pragma once

#include <gtest/gtest.h>

#include <stm32/cpu_register_set.hpp>

TEST(cpu, APSR_alignment)
{
    ASSERT_EQ(sizeof(stm32::ApplicationProgramStatusRegister), 4);

    stm32::ApplicationProgramStatusRegister APSR{};

    APSR.Z = true;
    ASSERT_EQ(*reinterpret_cast<uint32_t*>(&APSR), static_cast<uint32_t>(0x1u << APSR.FlagZBitNumber));
}
