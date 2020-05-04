#pragma once

#include <cstdint>

namespace stm32::rg
{
/**
 * Interrupt Set-Enable Register
 *
 * Enables, or reads the enable state of a group of interrupts
 *
 * NVIC_ISERn[31:0] are the set-enable bits for interrupts (31+(32*n)) - (32*n). When n=15, bits[31:16] are reserved
 */
struct __attribute__((packed)) InterruptSetEnableRegister {
    uint32_t SETENA : 32;  ///< bits[31:0]
                           ///< For register NVIC_ISERn, enables or shows the current enabled state of interrupt (m+(32*n)):
                           ///< When 0: on reads - interrupt disabled, on writes - no effect
                           ///< When 1: on reads - interrupt enabled, on writes - enable interrupt
};

/**
 * Interrupt Clear-Enable Register
 *
 * Disables, or reads the enable state of, a group of registers
 *
 * NVIC_ICERn[31:0] are the clear-enable bits for interrupts (31+(32*n)) - (32*n). When n=15, bits[31:16] are reserved
 */
struct __attribute__((packed)) InterruptClearEnableRegister {
    uint32_t CLRENA : 32;  ///< bits[31:0]
                           ///< For register NVIC_ICERn, disables or shows the current enabled state of interrupt (m+(32*n)):
                           ///< When 0: on reads - interrupt disabled, on writes - no effect
                           ///< When 1: on reads - interrupt enabled, on writes - disable interrupt
};

/**
 * Interrupt Set-Pending Register
 *
 * For a group of interrupts, changes interrupt status to pending, or shows the current pending status
 *
 * NVIC_ISPRn[31:0] are the set-pending bits for interrupts (31+(32*n)) - (32*n). When n=15, bits[31:16] are reserved.
 */
struct __attribute__((packed)) InterruptSetPendingRegister {
    uint32_t SETPEND : 32;  ///< bits[31:0]
                            ///< For register NVIC_ISPRn, changes the state of interrupt (m+(32*n)) to pending, or shows
                            ///< whether the state of the interrupt is pending:
                            ///< When 0: on reads - interrupt is not pending, on writes - no effect
                            ///< When 1: on reads - interrupt is pending, on writes - change state of interrupt to pending
};

/**
 * Interrupt Clear-Pending Register
 *
 * For a group of interrupts, clears the interrupt pending status, or shows the current pending status
 *
 * NVIC_ICPRn[31:0] are the clear-pending bits for interrupts (31+(32*n)) - (32*n). When n=15, bits[31:16] are reserved.
 */
struct __attribute__((packed)) InterruptClearPendingRegister {
    uint32_t CLRPEND : 32;  ///< bits[31:0]
                            ///< For register NVIC_ICPRn, clears the pending state of interrupt (m+(32*n)), or shows
                            ///< whether the state of the interrupt is pending:
                            ///< When 0: on reads - interrupt is not pending, on writes- no effect
                            ///< When 1: on reads - interrupt is pending, on writes - clears the pending state of the interrupt
};

/**
 * Interrupt Active Bit Register
 *
 * For a group of 32 interrupts, shows whether each interrupt is active.
 *
 * NVIC_IABRn[31:0] are the active bits for interrupts (31+(32*n)) - (32*n). When n=15, bits[31:16] are reserved.
 */
struct __attribute__((packed)) InterruptActiveBitRegister {
    uint32_t ACTIVE : 32;  ///< bits[31:0]
                           ///< For register NVIC_IABRn, shows whether interrupt (m+(32*n)) is active:
};

/**
 * Interrupt Priority Register
 *
 * Sets or reads interrupts priorities
 */
struct __attribute__((packed)) InterruptPriorityRegister {
    uint8_t PRI[4];
};

}  // namespace stm32::sc
