#pragma once
#include "stdjk/CoreHeader.h"
#include "stdjk/Complt.h"
#include "stdjk/Mem/Allocator.h"

namespace mem {

// Take from stdgcc
constexpr void Copy(void* Dest, const void* Src, USize Size) {
    Byte* d = Cast<Byte*>(Dest);
    const Byte* s = Cast<const Byte*>(Src);
    while (Size--)
        *d++ = *s++;
}

constexpr void Set(void* Dest, Byte Val, USize Size) {
    Byte* d = Cast<Byte*>(Dest);
    while (Size--) {
        *d++ = Val;
    }
}

constexpr void Move(void* Dest, const void* Src, USize Size) {
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

constexpr bool Cmp(void* Src1, void* Src2, USize Size) {
    const Byte* s1 = (const Byte*)Src1;
    const Byte* s2 = (const Byte*)Src2;

    while (Size--) {
        if (*s1 != *s2)
            return false;
    }

    return true;
}

template<typename T, typename... TArgs>
T* New(TArgs&&... Args) {
    T* ptr = Cast<T*>(
        Allocate(sizeof(T))
    );
    using ConstructorType = decltype(T::New(Args...));
    ConstructorType* ctor = Cast<ConstructorType*>(ptr);
    *ctor = ConstructorType::New(Args...);
    return ptr;
}

template<typename T>
void Destroy(T& Value) {
    if constexpr (IsPointer<T>) {
        if(Value)
            Value->Destroy();
        mem::Deallocate(Value);
    }
    else {
        Value.Destroy();
    }
}

}
