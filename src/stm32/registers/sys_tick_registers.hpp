#pragma once

#include <cstdint>

namespace stm32::rg
{
/**
 * SysTick Control and Status Register
 *
 * Controls the system timer and provides status data
 */
struct __attribute__((__packed__)) SysTickControlAndStatusRegister {
    bool ENABLE : 1;  ///< bit[0]
    ///< Indicates the enabled status of the SysTick counter
    ///< true - counter is operating
    ///< false - counter is disabled

    bool TICKINT : 1;  ///< bit[1]
    ///< Indicates whether counting to 0 causes the status of the SysTick exception to change to
    ///< pending
    ///< true - count to 0 changes the SysTick exception status to pending
    ///< false - count to 0 does not affect the SysTick exception status

    uint8_t CLKSOURCE : 1;  ///< bit[2]
    ///< Indicates the SysTick clock source:
    ///< 1 - SysTick uses the processor clock
    ///< 0 - SysTick uses the IMPLEMENTATION DEFINED external reference clock

    uint16_t : 13;  ///< bits[15:3]
    ///< Reserved

    bool COUNTFLAG : 1;  ///< bit[16]
    ///< Indicates whether the counter has counted to 0 since the last read of this register
    ///< true - timer has counted to 0
    ///< false - timer has not counted to 0
    ///< Note: COUNTFLAG is set to 1 by a count transition from 1 to 0
    ///< Note: COUNTFLAG is cleared to 0 by a software read of this register, and by and write to the
    ///< Current Value register
    ///< This bit is read only!

    uint16_t : 15;  ///< bits[31:17]
    ///< Reserved
};

/**
 * SysTick Reload Value Register
 *
 * Holds the reload value of the SYST_CVR
 */
struct __attribute__((__packed__)) SysTickReloadValueRegister {
    uint32_t RELOAD : 24;  ///< bits[23:0]
    ///< The value to load into the SYST_CVR when the counter reaches 0.

    uint8_t : 8;  ///< bits[31:24]
    ///< Reserved (read - all zero)
};

/**
 * SysTick Current Value Register
 *
 * Reads or clears the current counter value
 */
struct __attribute__((__packed__)) SysTickCurrentValueRegister {
    uint32_t CURRENT : 32;  ///< bits[31:0]
    ///< Current counter value
    ///< This is the value of the counter at the time it is sampled
};

/**
 * SysTick Calibration value Register
 *
 * Reads the calibration value and parameters for SysTick
 */
struct __attribute__((__packed__)) SysTickCalibrationValueRegister {
    uint32_t TENMS : 24;  ///< bits[23:0]
    ///< Optionally, holds a reload value to be used for 10ms (100Hz) timing, subject to system
    ///< clock skew errors. If this field is zero, the calibration value is not known.

    uint8_t : 6;  ///< bits[29:24]
    ///< Reserved

    bool SKEW : 1;  ///< bit[30]
    ///< Indicated whether the 10ms calibration value is exact
    ///< true - 10ms calibration value is INEXACT, because of the clock frequency
    ///< false - 10ms calibration value is exact

    bool NOREF : 1;  ///< bit[31]
    ///< Indicated whether the IMPLEMENTATION DEFINED reference clock is implemented:
    ///< true - the reference clock is not implemented
    ///< false - the reference clock is implemented
};

}  // namespace stm32::sc
