#pragma once
#include "jkr/CoreTypes.h"

constexpr USize Align(const USize N, const USize Alignment) {
    return (N + Alignment - 1) & ~(Alignment - 1);
}
