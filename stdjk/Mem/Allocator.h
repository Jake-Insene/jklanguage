#pragma once
#include "stdjk/Mem/Align.h"

namespace mem {

[[nodiscard]] Address AllocateAlign(USize Size, USize Alignment);

static inline [[nodiscard]] Address Allocate(USize Size) {
    return AllocateAlign(Size, sizeof(USize));
}

void Deallocate(Address Mem);

template<typename T>
static inline T* CountOf(USize Count) {
    return Cast<T*>(Allocate(sizeof(T) * Count));
}

}
