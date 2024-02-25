#pragma once
#include "stdjk/Mem/Allocator.h"

namespace mem {

constexpr USize PageSize = Platform == Windows ? 1024 * 4 
                           : 0;

constexpr USize AlignPage(USize N) {
    return Align(N, PageSize);
}

static inline [[nodiscard]] Address AllocatePage(USize Size) {
    return AllocateAlign(Size, PageSize);
}

}
