#pragma once
#include "jkr/CoreHeader.h"

#include <initializer_list>
#include <assert.h>

template<typename T>
struct Slice {
    constexpr Slice() 
        : Data(nullptr), Len(0)
    {}

    constexpr Slice(const T* _Data, Uint64 Len) noexcept :
        Data(_Data), Len(Len)
    {}

    constexpr Slice(const std::initializer_list<T>& Args) noexcept :
        Data(Args.begin()), Len(Args.end() - Args.begin())
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

    constexpr const T& operator[](Uint64 Index) const noexcept {
        assert(Index < Len && "Index out of range");
        return Data[Index];
    }

    constexpr const T* begin() const noexcept { return &Data[0]; }
    constexpr const T* end() const noexcept { return &Data[Len]; }

    const T* Data = nullptr;
    Uint64 Len = 0;
};
