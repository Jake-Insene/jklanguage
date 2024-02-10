#pragma once
#include "jkr/Mem/Allocator.h"
#include "jkr/Lib/Debug.h"
#include "jkr/Lib/Error.h"

namespace mem {

template<typename T>
struct [[nodiscard]] Ptr {

    static constexpr Ptr New(T* P) {
        return Ptr{
            .IsValid = P ? true : false,
            .Data = P,
        };
    }

    [[nodiscard]] constexpr T* operator->(this Ptr& Self) {
        DebugAssert(Self.IsValid);
        RuntimeError(Self.IsValid, STR("Invalid pointer"));
        return Self.Data; 
    }

    [[nodiscard]] constexpr operator bool(this Ptr& Self) { return Self.IsValid; }

    void Destroy(this Ptr& Self) {
        if (Self.IsValid) {
            Self.Data->Destroy();
            Deallocate(Cast<Address>(Self.Data), sizeof(T));
            Self.IsValid = false;
        }
    }

    bool IsValid = false;
    T* Data = nullptr;
};

template<typename T, typename... TArgs>
Ptr<T> New(TArgs&&... Args) {
    using SubType = decltype(T::New(Args...));
    T* p = Cast<T*>(Allocate(sizeof(T)));
    SubType* s = (SubType*)p;
    *s = T::New(Args...);
    return Ptr<T>::New(p);
}

}
