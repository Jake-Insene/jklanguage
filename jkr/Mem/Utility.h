#pragma once
#include "jkr/CoreHeader.h"

namespace mem {

// Take from stdgcc
template<typename T>
constexpr void Copy(T* Dest, const T* Src, USize Size) {
    Byte* d = Cast<Byte*>(Dest);
    const Byte* s = Cast<const Byte*>(Src);
    while (Size--)
        *d++ = *s++;
}

template<typename T>
constexpr void Set(T* Dest, Byte Val, USize Size) {
    Byte* d = Cast<Byte*>(Dest);
    while (Size--) {
        *d++ = Val;
    }
}

template<typename T>
constexpr void Move(T* Dest, const T* Src, USize Size) {
    Byte* d = Cast<Byte*>(Dest);
    const Byte* s = Cast<const Byte*>(Src);
    if (d < s)
        while (Size--)
            *d++ = *s++;
    else {
        const Byte* lasts = s + (Size - 1);
        Byte* lastd = d + (Size - 1);
        while (Size--)
            *lastd-- = *lasts--;
    }
}

}
