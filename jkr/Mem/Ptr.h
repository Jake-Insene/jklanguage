#pragma once
#include "jkr/Mem/Allocator.h"
#include "jkr/Lib/Debug.h"
#include "jkr/Lib/Error.h"
#include "jkr/Lib/Complt.h"

namespace mem {

template<typename T, bool UseNew = true>
struct [[nodiscard]] Ptr {

    static constexpr Ptr New(T* P) {
        return Ptr{
            .IsValid = P ? true : false,
            .Data = P,
        };
    }

    template<typename T>
    static constexpr Ptr FromPtr(const Ptr<T>& P) {
        return Ptr{
            .IsValid = P.IsValid,
            .Data = P.Data,
        };
    }

    [[nodiscard]] constexpr T* operator->(this Ptr& Self) {
        RuntimeError(Self.IsValid, STR("Invalid pointer"));
        return Self.Data;
    }

    [[nodiscard]] constexpr operator bool(this Ptr& Self) { return Self.IsValid; }

    void Destroy(this Ptr& Self) {
        if (Self.IsValid) {
            if constexpr (UseNew && !IsPrimitive<T>) {
                Self.Data->Destroy();
            }
            Deallocate(Self.Data);
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
