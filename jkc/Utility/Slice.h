#pragma once
#include "stdjk/CoreHeader.h"
#include "stdjk/Error.h"

template<typename T>
struct Slice {
    constexpr Slice() 
        : Data(nullptr), Len(0)
    {}

    constexpr Slice(const T* _Data, UInt64 Len) noexcept :
        Data(_Data), Len(Len)
    {}

    constexpr Slice(const Slice& RHS) noexcept :
        Data(RHS.Data), Len(RHS.Len)
    {}

    constexpr Slice(Slice&& RHS) noexcept :
        Data(RHS.Data), Len(RHS.Len)
    {
        RHS.Data = nullptr;
        RHS.Len = 0;
    }

    constexpr Slice& operator=(const Slice& RHS) noexcept {
        Data = RHS.Data;
        Len = RHS.Len;
        return *this;
    }

    constexpr Slice& operator=(Slice&& RHS) noexcept {
        Data = RHS.Data;
        Len = RHS.Len;

        RHS.Data = nullptr;
        RHS.Len = 0;
        return *this;
    }

    constexpr ~Slice() noexcept {}

    constexpr const T& operator[](UInt64 Index) const noexcept {
        RuntimeError(Index < Len, STR("Index out of range"));
        return Data[Index];
    }

    constexpr const T* begin() const noexcept { return &Data[0]; }
    constexpr const T* end() const noexcept { return &Data[Len]; }

    const T* Data = nullptr;
    UInt64 Len = 0;
};
