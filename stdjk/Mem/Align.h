#pragma once
#include "stdjk/CoreHeader.h"

namespace mem {

constexpr USize Align(const USize N, const USize Alignment) {
    return (N + Alignment - 1) & ~(Alignment - 1);
}

}
