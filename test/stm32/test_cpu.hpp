#pragma once

#include <gtest/gtest.h>

#include <stm32/cpu_register_set.hpp>
#include <stm32/memory.hpp>
#include <stm32/opcodes.hpp>

#include "utils.hpp"

TEST(cpu, Opcodes)
{
    stm32::CpuRegisterSet registers{};
    auto memory = details::createMemory();

    stm32::opcodes::cmdShiftImmediate<stm32::opcodes::Encoding::T1, stm32::math::ShiftType::LSR>(uint16_t{}, registers, memory);
}
