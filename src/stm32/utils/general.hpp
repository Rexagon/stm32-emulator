#pragma once

#include <cassert>

#include "exceptions.hpp"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define UNPREDICTABLE throw ::stm32::utils::UnpredictableException("unconditional" __FILE__ " " TOSTRING(__LINE__))
#define UNPREDICTABLE_IF(expression) \
    if (expression)                  \
        throw ::stm32::utils::UnpredictableException { "conditional " #expression }

#define UNDEFINED throw ::stm32::utils::UndefinedException("undefined" __FILE__ " " TOSTRING(__LINE__))
#define UNDEFINED_IF(expression) \
    if (expression)              \
    throw ::stm32::utils::UndefinedException("undefined" __FILE__ " " TOSTRING(__LINE__))

#define UNIMPLEMENTED assert(false)

#define UNUSED(x) (void)(x)

#define DEFINE_REG(StructName, definition)         \
    union StructName {                             \
        struct __attribute__((packed)) definition; \
        uint32_t registerData;                     \
    }

#define DEFINE_HALFREG(StructName, definition)     \
    union StructName {                             \
        struct __attribute__((packed)) definition; \
        uint16_t registerData;                     \
    }

#define DEFINE_BYTEREG(StructName, definition)     \
    union StructName {                             \
        struct __attribute__((packed)) definition; \
        uint8_t registerData;                      \
    }

#define RESERVE_BYTE(bits) \
    uint8_t:               \
    bits
#define RESERVE_HW(bits) \
    uint16_t:            \
    bits
#define RESERVE(bits) \
    uint32_t:         \
    bits

namespace stm32::utils
{
template <typename T, typename S = int>
class ArrayView {
public:
    ArrayView(T* ptr, S size) noexcept
        : m_ptr{ptr}
        , m_size{size}
    {
    }

    auto operator[](S i) noexcept -> T& { return m_ptr[i]; }
    auto operator[](S i) const noexcept -> const T& { return m_ptr[i]; }

    inline auto size() const noexcept { return m_size; }

    inline auto begin() noexcept { return m_ptr; }
    inline auto end() noexcept { return m_ptr + m_size; }

private:
    T* m_ptr;
    S m_size;
};

}  // namespace utils
