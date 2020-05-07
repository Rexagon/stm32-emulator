#pragma once

#include <cassert>

#define UNPREDICTABLE assert(false)
#define UNPREDICTABLE_IF(expression) assert(!(expression))

#define DEFINE_REG(StructName, definition) union StructName { \
    struct __attribute__((packed)) definition; \
    uint32_t registerData; \
}

#define DEFINE_HALFREG(StructName, definition) union StructName { \
    struct __attribute__((packed)) definition; \
    uint16_t registerData; \
}

#define DEFINE_BYTEREG(StructName, definition) union StructName { \
    struct __attribute__((packed)) definition; \
    uint8_t registerData; \
}

#define RESERVE_BYTE(bits) uint8_t : bits
#define RESERVE_HW(bits) uint16_t : bits
#define RESERVE(bits) uint32_t : bits
