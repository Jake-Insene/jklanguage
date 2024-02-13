#pragma once
#include "jkr/CoreHeader.h"

namespace mem {
    [[nodiscard]] Address JK_API Allocate(USize Size);

    void JK_API Deallocate(Address Mem);

    template<typename T>
    static inline T* CountOf(USize Count) {
        return Cast<T*>(Allocate(sizeof(T)*Count));
    }

}
