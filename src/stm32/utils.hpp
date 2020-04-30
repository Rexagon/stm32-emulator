#pragma once

#include <cassert>

#define UNPREDICTABLE assert(false)
#define UNPREDICTABLE_IF(expression) assert(!(expression))
