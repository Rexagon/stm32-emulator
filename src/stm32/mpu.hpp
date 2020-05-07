#pragma once

#include "registers/mpu_registers_set.hpp"

namespace stm32
{
class Cpu;

enum class MemoryType : uint8_t {
    Normal = 0b00u,
    Device = 0b01u,
    StronglyOrdered = 0b10u,
};

enum class CacheAttribute : uint8_t {
    NonCacheable = 0b00u,
    WBWA = 0b01u,   //  Write-back, write and read allocate
    WT = 0b10u,     // Write-through, no write allocate
    WBnWA = 0b11u,  // Write-back, no write allocate
};

// Memory attributes descriptor
struct __attribute__((__packed__)) MemoryAttributes {
    MemoryType type : 2;       // bits[1:0]
    CacheAttribute inner : 2;  // bits[3:2]
    CacheAttribute outer : 2;  // bits[5:4]
    bool shareable : 1;        // bit[6]

    uint8_t : 1;  // bit[7]
    // reserved
};

// Descriptor used to access the underlying memory array
struct AddressDescriptor {
    MemoryAttributes attributes;
    uint32_t physicalAddress;
};

struct MemoryPermissions {
    uint8_t accessPermissions;
    bool executeNever;
};

enum AccessType {
    Normal,
    Unprivileged,
    VecTable,
    InstructionFetch,
};

class Mpu {
public:
    explicit Mpu(Cpu& cpu);

    void reset();

    template <typename T>
    auto alignedMemoryRead(uint32_t address, AccessType accessType = AccessType::Normal) -> T;
    template <typename T>
    void alignedMemoryWrite(uint32_t address, T value, AccessType accessType = AccessType::Normal);

    auto validateAddress(uint32_t address, AccessType accessType, bool write) -> AddressDescriptor;
    void checkPermissions(MemoryPermissions permissions, uint32_t address, AccessType accessType, bool write);

    inline auto registers() -> rg::MpuRegistersSet& { return m_registers; }

    static auto defaultMemoryAttributes(uint32_t address) -> MemoryAttributes;
    static auto defaultMemoryPermissions(uint32_t address) -> MemoryPermissions;
    static auto defaultTexDecode(const rg::MpuRegionAttribute& attributes) -> MemoryAttributes;

private:
    Cpu& m_cpu;
    rg::MpuRegistersSet m_registers;
};

}  // namespace stm32
