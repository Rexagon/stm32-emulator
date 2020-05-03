#pragma once

#include "mpu_registers.hpp"

namespace stm32::sc
{
class MpuRegistersSet {
public:
    explicit MpuRegistersSet();

    void reset();

private:
};

}  // namespace stm32::sc
