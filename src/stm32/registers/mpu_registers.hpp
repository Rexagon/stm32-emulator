#pragma once

#include <cstdint>

#include "../utils/general.hpp"

namespace stm32::rg
{
/**
 * MPU Type Register
 *
 * The MPU Type Register indicates how many regions the MPU support. Software can use it to determine if the processor implements an MPU.
 */
DEFINE_REG(MpuTypeRegister, {
    bool SEPARATE : 1;  ///< bit[0]
                        ///< Indicates support for separate instruction and data address maps. RAZ. ARMv7-M only supports a unified MPU.

    RESERVE(7);  ///< bits[7:1]

    uint8_t DREGION : 8;  ///< bits[15:8]
                          ///< Number of regions supported by the MPU. If this field reads-as-zero the processor does not implement an MPU.

    uint8_t IREGION : 8;  ///< bits[24:16]
                          ///< Instruction region. RAZ. ARMv7-M only supports a unified MPU.

    RESERVE(8);  ///< bits[31:24]
});

/**
 * MPU Control Register
 *
 * Enables the MPU, and when the MPU is enabled, controls whether the default memory map is enabled as a background region for privileged
 * accesses, and whether the MPU is enabled for HardFaults, NMIs, and exception handlers when FAULTMASK is set to 1.
 */
DEFINE_REG(MpuControlRegister, {
    bool ENABLE : 1;  ///< bit[0]
                      ///< Enables the MPU

    bool HFNMIENA : 1;  ///< bit[1]
                        ///< When the ENABLE bit is set to 1, controls whether handlers executing with priority less
                        ///< than 0 access memory with the MPU enabled or with the MPU disabled. This applies to
                        ///< HardFaults, NMIs, and exception handlers when FAULTMASK is set to 1:
                        ///<    When 0: disables the MPU for these handlers.
                        ///<    When 1: Use the MPU for memory accesses by these handlers.
                        ///<
                        ///< \note If HFNMIENA is set to 1 when ENABLE is set to 0, behavior is UNPREDICTABLE

    bool PRIVDEFENA : 1;  ///< bit[2]
                          ///< When the ENABLE bit is set to 1, the meaning of this bit is:
                          ///<    When 0: Disables the default memory map. Any instruction or data access that does not access a defined
                          ///<        region faults.
                          ///<    When 1: Enables the default memory map as a background region for privileged access. The background region
                          ///<        acts as region number -1. All memory regions configured in the MPU take priority over the default
                          ///<        memory map

    RESERVE(29);  ///< bits[31:3]
});

/**
 * MPU Region Number Register
 *
 * Selects the region currently accessed by MPU_RBAR and MPU_RASR
 */
DEFINE_REG(MpuRegionNumberRegister, {
    uint8_t REGION : 8;  ///< bits[7:0]
                         ///< Indicates the memory region accessed by MPU_RBAR and MPU_RASR.

    RESERVE(24);  ///< bits[31:8]
});

/**
 * MPU Region Base Address Register
 *
 * Holds the base address of the region identified by MPU_RNR. On a write, can also be used to update the base address of a specified
 * region, in the range 0 to 15, updating MPU_RNR with the new region number
 */
DEFINE_REG(MpuRegionBaseAddressRegister, {
    uint8_t REGION : 4;  ///< bits[3:0]
                         ///< On writes, can specify the number of the region to update, see VALID field description
                         ///< On reads, returns bits[3:0] of MPU_RNR.

    bool VALID : 1;  ///< bit[4]
                     ///< On writes, indicates whether the region to update is specified by MPU_RNR.REGION, or by the REGION value specified
                     ///< in this write. When using the REGION value specified by this write, MPU_RNR.REGION is updated to this value.
                     ///<
                     ///< When 0: Apply the base address update to the region specified by MPU_RNR.REGION. The REGION field value is
                     ///< ignored.
                     ///< When 1: Update MPU_RNR.REGION to the value obtained by zero extending the REGION value specified in this
                     ///< write, and apply the base address update to this region.
                     ///<
                     ///< \note This bit reads as zero

    uint32_t ADDR : 27;  ///< bits[31:5]
});

/**
 * MPU Region Attribute field
 */
DEFINE_HALFREG(MpuRegionAttribute, {
    bool B : 1;  ///< bit[16]

    bool C : 1;  ///< bit[17]

    bool S : 1;  ///< bit[18]

    uint8_t TEX : 3;  ///< bits[21:19]

    RESERVE_HW(2);  ///< bits[23:22]

    uint8_t AP : 3;  ///< bits[26:24]

    RESERVE_HW(1);  ///< bit[27]

    bool XN : 1;  ///< bit[28]

    RESERVE_HW(3);  ///< bits[31:29]
});

/**
 * MPU Region Attribute and Size Register
 *
 * Defines the size and access behavior of the region identified by MPU_RNR, and enables that region.
 */
DEFINE_REG(MpuRegionAttributeAndSizeRegister, {
    bool ENABLE : 1;  ///< bit[0]
                      ///< Enables this region
                      ///< \note Enabling a region has no effect unless the MPU_CTRL.ENABLE bit is set to 1, to enable the MPU.

    uint8_t SIZE : 5;  ///< bits[5:1]
                       ///< Indicates the region size. The region size, in bytes, is 2(SIZE+1). SIZE field values less than 4 are reserved,
                       ///< because the smallest supported region size is 32 bytes.

    RESERVE(2);  ///< bits[7:6]

    uint8_t SRD : 8;  ///< bits[15:8]
                      ///< Subregion Disable. For regions of 256 bytes or larger, each bit of this field controls whether one of the eight
                      ///< equal subregions is enabled

    MpuRegionAttribute ATTRS;  ///< bits[31:16]
});

}  // namespace stm32::rg
