#pragma once

#include "cpu_register_set.hpp"
#include "memory.hpp"

namespace stm32::opcodes::t1
{
void cmd_lsl(uint16_t opCode, CpuRegisterSet& registers, Memory& memory);
void cmd_lsr(uint16_t opCode, CpuRegisterSet& registers, Memory& memory);
void cmd_asr(uint16_t opCode, CpuRegisterSet& registers, Memory& memory);
void cmd_add(uint16_t opCode, CpuRegisterSet& registers, Memory& memory);
void cmd_sub(uint16_t opCode, CpuRegisterSet& registers, Memory& memory);
void cmd_mov(uint16_t opCode, CpuRegisterSet& registers, Memory& memory);
void cmd_cmp(uint16_t opCode, CpuRegisterSet& registers, Memory& memory);

}  // namespace stm32::opcodes::t1
