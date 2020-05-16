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
