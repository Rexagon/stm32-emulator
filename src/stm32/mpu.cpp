// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "mpu.hpp"

#include "cpu.hpp"
#include "utils/math.hpp"

namespace stm32
{
using namespace utils;

namespace details
{
template <typename T>
inline auto alignedMemoryRead(Cpu& cpu, Mpu& mpu, uint32_t address, AccessType accessType) -> T
{
    if (!isAddressAligned<T>(address)) {
        cpu.systemRegisters().CFSR().usageFault.UNALIGNED_ = true;
        throw utils::CpuException(ExceptionType::UsageFault);
    }

    const auto descriptor = mpu.validateAddress(address, accessType, false);
    auto value = cpu.memory().read<T>(descriptor.physicalAddress);

    if (cpu.systemRegisters().AIRCR().ENDIANNESS) {
        value = reverseEndianness(value);
    }

    return value;
}

template <typename T>
inline void alignedMemoryWrite(Cpu& cpu, Mpu& mpu, uint32_t address, T value, AccessType accessType)
{
    if (!isAddressAligned<T>(address)) {
        cpu.systemRegisters().CFSR().usageFault.UNALIGNED_ = true;
        throw utils::CpuException(ExceptionType::UsageFault);
    }

    const auto descriptor = mpu.validateAddress(address, accessType, true);

    if (cpu.systemRegisters().AIRCR().ENDIANNESS) {
        value = reverseEndianness(value);
    }

    cpu.memory().write<T>(descriptor.physicalAddress, value);
}

template <typename T>
inline auto unalignedMemoryRead(Cpu& cpu, Mpu& mpu, uint32_t address, AccessType accessType) -> T
{
    if (isAddressAligned<T>(address)) {
        return alignedMemoryRead<T>(cpu, mpu, address, accessType);
    }
    else if (cpu.systemRegisters().CCR().UNALIGN_TRP) {
        cpu.systemRegisters().CFSR().usageFault.UNALIGNED_ = true;
        throw utils::CpuException(ExceptionType::UsageFault);
    }

    T value{};
    for (uint8_t i = 0; i < sizeof(T); ++i) {
        value = value | static_cast<T>(static_cast<T>(alignedMemoryRead<uint8_t>(cpu, mpu, address, accessType)) << (8u * i));
    }
    if (cpu.systemRegisters().AIRCR().ENDIANNESS) {
        value = reverseEndianness(value);
    }

    return value;
}

template <typename T>
inline void unalignedMemoryWrite(Cpu& cpu, Mpu& mpu, uint32_t address, T value, AccessType accessType)
{
    if (isAddressAligned<T>(address)) {
        alignedMemoryWrite<T>(cpu, mpu, address, value, accessType);
        return;
    }

    else if (cpu.systemRegisters().CCR().UNALIGN_TRP) {
        cpu.systemRegisters().CFSR().usageFault.UNALIGNED_ = true;
        throw utils::CpuException(ExceptionType::UsageFault);
    }

    if (cpu.systemRegisters().AIRCR().ENDIANNESS) {
        value = reverseEndianness(value);
    }

    for (uint8_t i = 0; i < sizeof(T); ++i) {
        alignedMemoryWrite<uint8_t>(cpu, mpu, address, static_cast<uint8_t>(value >> (8u * i)), accessType);
    }
}

}  // namespace details

Mpu::Mpu(Cpu& cpu)
    : m_cpu{cpu}
    , m_registers{}
{
}

void Mpu::reset()
{
    m_registers.reset();
}

template <>
auto Mpu::alignedMemoryRead<uint8_t>(uint32_t address, AccessType accessType) -> uint8_t
{
    return details::alignedMemoryRead<uint8_t>(m_cpu, *this, address, accessType);
}

template <>
auto Mpu::alignedMemoryRead<uint16_t>(uint32_t address, AccessType accessType) -> uint16_t
{
    return details::alignedMemoryRead<uint16_t>(m_cpu, *this, address, accessType);
}

template <>
auto Mpu::alignedMemoryRead<uint32_t>(uint32_t address, AccessType accessType) -> uint32_t
{
    return details::alignedMemoryRead<uint32_t>(m_cpu, *this, address, accessType);
}

template <>
void Mpu::alignedMemoryWrite(uint32_t address, uint8_t value, AccessType accessType)
{
    return details::alignedMemoryWrite<uint8_t>(m_cpu, *this, address, value, accessType);
}

template <>
void Mpu::alignedMemoryWrite(uint32_t address, uint16_t value, AccessType accessType)
{
    return details::alignedMemoryWrite<uint16_t>(m_cpu, *this, address, value, accessType);
}

template <>
void Mpu::alignedMemoryWrite(uint32_t address, uint32_t value, AccessType accessType)
{
    return details::alignedMemoryWrite<uint32_t>(m_cpu, *this, address, value, accessType);
}

template <>
auto Mpu::unalignedMemoryRead<uint8_t>(uint32_t address, AccessType accessType) -> uint8_t
{
    return details::unalignedMemoryRead<uint8_t>(m_cpu, *this, address, accessType);
}

template <>
auto Mpu::unalignedMemoryRead<uint16_t>(uint32_t address, AccessType accessType) -> uint16_t
{
    return details::unalignedMemoryRead<uint16_t>(m_cpu, *this, address, accessType);
}

template <>
auto Mpu::unalignedMemoryRead<uint32_t>(uint32_t address, AccessType accessType) -> uint32_t
{
    return details::unalignedMemoryRead<uint32_t>(m_cpu, *this, address, accessType);
}

template <>
void Mpu::unalignedMemoryWrite(uint32_t address, uint8_t value, AccessType accessType)
{
    return details::unalignedMemoryWrite<uint8_t>(m_cpu, *this, address, value, accessType);
}

template <>
void Mpu::unalignedMemoryWrite(uint32_t address, uint16_t value, AccessType accessType)
{
    return details::unalignedMemoryWrite<uint16_t>(m_cpu, *this, address, value, accessType);
}

template <>
void Mpu::unalignedMemoryWrite(uint32_t address, uint32_t value, AccessType accessType)
{
    return details::unalignedMemoryWrite<uint32_t>(m_cpu, *this, address, value, accessType);
}

auto Mpu::validateAddress(uint32_t address, AccessType accessType, bool write) -> AddressDescriptor
{
    const auto isPrivileged = accessType != AccessType::Unprivileged && m_cpu.isInPrivilegedMode();

    AddressDescriptor result{};
    result.physicalAddress = address;
    result.attributes = defaultMemoryAttributes(address);

    auto permissions = defaultMemoryPermissions(address);

    auto hit = false;

    auto isPpbAccess = getPart<20, 12, uint16_t>(address) == 0b111000000000u;
    if (accessType == AccessType::VecTable || isPpbAccess) {
        hit = true;  // // use default map for PPB and vector table lookups
    }
    else if (!m_registers.MPU_CTRL().ENABLE) {
        UNPREDICTABLE_IF(m_registers.MPU_CTRL().HFNMIENA);
        hit = true;  // always use default map if MPU disabled
    }
    else if (!m_registers.MPU_CTRL().HFNMIENA && m_cpu.executionPriority() < 0) {
        hit = true;  // optionally use default for HardFault, NMI and FAULTMASK
    }
    else {  // MPU is enabled so check each individual region
        if (m_registers.MPU_CTRL().PRIVDEFENA && isPrivileged) {
            hit = true;  // optional default as background for Privileged accesses
        }

        // highest matching region wins
        // TODO: reverse loop iteration
        for (uint8_t r = 0; r < m_registers.MPU_TYPE().DREGION; ++r) {
            const auto& MPU_RBAR = m_registers.MPU_RBAR(r);
            const auto& MPU_RASR = m_registers.MPU_RASR(r);

            if (!MPU_RASR.ENABLE) {
                continue;
            }

            // MPU region enabled so perform checks
            const auto lsBit = MPU_RASR.SIZE + 1u;
            UNPREDICTABLE_IF(lsBit < 5u);
            UNPREDICTABLE_IF((lsBit < 8u) && (MPU_RASR.ATTRS.registerData != 0u));

            if (lsBit == 32u || (address >> lsBit) == (MPU_RBAR.registerData >> lsBit)) {
                const auto subRegion = (address >> (lsBit - 3u)) & 0b111u;
                if ((static_cast<uint8_t>(MPU_RASR.SRD >> subRegion) & 0b1u) == 0u) {
                    permissions.accessPermissions = MPU_RASR.ATTRS.AP;
                    permissions.executeNever = MPU_RASR.ATTRS.XN;
                    result.attributes = defaultTexDecode(MPU_RASR.ATTRS);
                    hit = true;
                }
            }
        }
    }

    if (getPart<29, 3>(address) == 0b111u) {
        permissions.executeNever = true;
    }

    if (hit) {
        checkPermissions(permissions, address, accessType, write);
    }
    else {
        if (accessType == AccessType::InstructionFetch) {
            m_cpu.systemRegisters().CFSR().memManage.IACCVIOL = true;
            m_cpu.systemRegisters().CFSR().memManage.MMARVALID = false;
        }
        else {
            m_cpu.systemRegisters().CFSR().memManage.DACCVIOL = true;
            m_cpu.systemRegisters().MMFAR().ADDRESS = address;
            m_cpu.systemRegisters().CFSR().memManage.MMARVALID = true;
        }

        throw utils::CpuException(ExceptionType::MemManage);
    }

    return result;
}

void Mpu::checkPermissions(MemoryPermissions permissions, uint32_t address, AccessType accessType, bool write)
{
    const auto isPrivileged = accessType != AccessType::Unprivileged && m_cpu.isInPrivilegedMode();

    bool fault{};
    switch (permissions.accessPermissions) {
        case 0b000u:
            fault = true;
            break;

        case 0b001u:
            fault = !isPrivileged;
            break;

        case 0b010u:
            fault = !isPrivileged && write;
            break;

        case 0b011u:
            fault = false;
            break;

        case 0b100u:
            UNPREDICTABLE;

        case 0b101u:
            fault = !isPrivileged || write;
            break;

        case 0b110u:
        case 0b111u:
            fault = write;
            break;

        default:
            UNPREDICTABLE;
    }

    if (accessType == AccessType::InstructionFetch) {
        if (fault || permissions.executeNever) {
            m_cpu.systemRegisters().CFSR().memManage.IACCVIOL = true;
            m_cpu.systemRegisters().CFSR().memManage.MMARVALID = false;
            throw utils::CpuException(ExceptionType::MemManage);
        }
    }
    else if (fault) {
        m_cpu.systemRegisters().CFSR().memManage.DACCVIOL = true;
        if (!m_cpu.systemRegisters().CFSR().memManage.MMARVALID) {
            m_cpu.systemRegisters().MMFAR().ADDRESS = address;
            m_cpu.systemRegisters().CFSR().memManage.MMARVALID = true;
        }
        throw utils::CpuException(ExceptionType::MemManage);
    }
}

auto Mpu::defaultMemoryAttributes(uint32_t address) -> MemoryAttributes
{
    MemoryAttributes attributes{};
    switch (getPart<29, 3>(address)) {
        case 0b000u:
            attributes.type = MemoryType::Normal;
            attributes.inner = CacheAttribute::WT;
            attributes.shareable = false;
            break;
        case 0b001u:
            attributes.type = MemoryType::Normal;
            attributes.inner = CacheAttribute::WBWA;
            attributes.shareable = false;
            break;
        case 0b010u:
            attributes.type = MemoryType::Device;
            attributes.inner = CacheAttribute::NonCacheable;
            attributes.shareable = false;
            break;
        case 0b011u:
            attributes.type = MemoryType::Normal;
            attributes.inner = CacheAttribute::WBWA;
            attributes.shareable = false;
            break;
        case 0b100u:
            attributes.type = MemoryType::Normal;
            attributes.inner = CacheAttribute::WT;
            attributes.shareable = false;
            break;
        case 0b101u:
            attributes.type = MemoryType::Device;
            attributes.inner = CacheAttribute::NonCacheable;
            attributes.shareable = true;
            break;
        case 0b110u:
            attributes.type = MemoryType::Device;
            attributes.inner = CacheAttribute::NonCacheable;
            attributes.shareable = false;
            break;
        case 0b111u:
            if (getPart<20, 8>(address) == 0u) {
                attributes.type = MemoryType::StronglyOrdered;
                attributes.inner = CacheAttribute::NonCacheable;
                attributes.shareable = true;
            }
            else {
                attributes.type = MemoryType::Device;
                attributes.inner = CacheAttribute::NonCacheable;
                attributes.shareable = false;
            }
            break;
    }

    attributes.outer = attributes.inner;
    return attributes;
}

auto Mpu::defaultMemoryPermissions(uint32_t address) -> MemoryPermissions
{
    MemoryPermissions permissions;
    permissions.accessPermissions = 0b011u;

    switch (getPart<29, 3>(address)) {
        case 0b000u:
            permissions.executeNever = false;
            break;
        case 0b001u:
            permissions.executeNever = false;
            break;
        case 0b010u:
            permissions.executeNever = true;
            break;
        case 0b011u:
            permissions.executeNever = false;
            break;
        case 0b100u:
            permissions.executeNever = false;
            break;
        case 0b101u:
            permissions.executeNever = true;
            break;
        case 0b110u:
            permissions.executeNever = true;
            break;
        case 0b111u:
            permissions.executeNever = true;
            break;
    }

    return permissions;
}

auto Mpu::defaultTexDecode(const rg::MpuRegionAttribute& attributes) -> MemoryAttributes
{
    MemoryAttributes result;

    const auto texcb = combine<uint8_t>(_<0>{attributes.B}, _<1>{attributes.C}, _<2, 3>{attributes.TEX});
    switch (texcb) {
        case 0b00000u:
            result.type = MemoryType::StronglyOrdered;
            result.inner = result.outer = CacheAttribute::NonCacheable;
            result.shareable = true;
            break;

        case 0b00001u:
            result.type = MemoryType::Device;
            result.inner = result.outer = CacheAttribute::NonCacheable;
            result.shareable = true;
            break;

        case 0b0001'0u ... 0b0001'1u:
        case 0b00100u:
            result.type = MemoryType::Normal;
            result.inner = result.outer = static_cast<CacheAttribute>(texcb & 0b11u);
            result.shareable = attributes.S;
            break;

        case 0b00110u:
            // TODO: implementation defined, check on real microcontroller
            break;

        case 0b00111u:
            result.type = MemoryType::Normal;
            result.inner = result.outer = CacheAttribute::WBWA;
            result.shareable = attributes.S;
            break;

        case 0b01000u:
            result.type = MemoryType::Device;
            result.inner = result.outer = CacheAttribute::NonCacheable;
            result.shareable = false;
            break;

        case 0b1'0000u ... 0b1'1111u:
            result.type = MemoryType::Normal;
            result.inner = static_cast<CacheAttribute>(getPart<0, 2>(texcb));
            result.outer = static_cast<CacheAttribute>(getPart<2, 2>(texcb));
            result.shareable = attributes.S;
            break;

        default:
            UNPREDICTABLE;
    }

    return result;
}

}  // namespace stm32
