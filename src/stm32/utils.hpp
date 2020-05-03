#pragma once

#include <cassert>

#define UNPREDICTABLE assert(false)
#define UNPREDICTABLE_IF(expression) assert(!(expression))

template <typename T, typename V = uint32_t>
void resetRegisterValue(T& reg, V value = V{0})
{
    static_assert(sizeof(T) == sizeof(V));
    *reinterpret_cast<V*>(&reg) = value;
}
