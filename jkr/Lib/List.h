#pragma once
#include "jkr/Mem/Allocator.h"
#include "jkr/Mem/Utility.h"
#include "jkr/Lib/Debug.h"
#include "jkr/Lib/Error.h"
#include "jkr/Lib/Complt.h"

template<typename T, bool UseNew = true>
struct [[nodiscard]] List {
    static constexpr bool IsPrimitive = IsSameAs<
        T, Byte, Int, UInt, USize, ISize, IntPtr, Address,
        char, short, int, long long,
        unsigned short, unsigned int, unsigned long long
    >;

    using Iterator = T*;
    using ConstIterator = const T*;

    static constexpr List New(USize InitialSize) {
        return List{
            .Capacity = InitialSize,
            .Size = 0,
            .Data = mem::CountOf<T>(InitialSize),
        };
    }

    [[nodiscard]] constexpr Iterator begin() { return Data; }
    [[nodiscard]] constexpr ConstIterator begin() const { return Data; }
    [[nodiscard]] constexpr Iterator end() { return &Data[Size]; }
    [[nodiscard]] constexpr ConstIterator end() const { return &Data[Size]; }

    [[nodiscard]] constexpr T& operator[](this List& Self, USize Index) {
        DebugAssert(Index < Self.Size);
        RuntimeError(Index < Self.Size, STR("Index out of range"));
        return Self.Data[Index];
    }

    [[nodiscard]] constexpr const T& operator[](this const List& Self, USize Index) {
        DebugAssert(Index < Self.Size);
        RuntimeError(Index < Self.Size, STR("Index out of range"));
        return Self.Data[Index];
    }

    [[nodiscard]] constexpr T& Get(this List& Self, USize Index) {
        DebugAssert(Index < Self.Size);
        RuntimeError(Index < Self.Size, STR("Index out of range"));
        return Self.Data[Index];
    }

    template<typename... TArgs>
    constexpr T& Push(this List& Self, TArgs&&... Args) {
        if (Self.Size >= Self.Capacity)
            Self.GrowCapacity(Self.Capacity ? Self.Capacity + (Self.Capacity/2) : 2);

        if constexpr (UseNew && !IsPrimitive) {
            return Self.Data[Self.Size++] = Cast<T&&>(T::New(Args...));
        }
        else if constexpr (IsPrimitive) {
            return Self.Data[Self.Size++] = T(Args...);
        }
        else {
            return Self.Data[Self.Size++];
        }
    }

    constexpr T& Insert(this List& Self, USize Index, T&& V) {
        if (Self.Size >= Self.Capacity)
            Self.GrowCapacity(Self.Capacity ? Self.Capacity + (Self.Capacity / 2) : 2);

        mem::Move(Self.Data + Index + 1, Self.Data + Index, sizeof(T) * Self.Size);
        if constexpr (UseNew) {
            Self.Data[Index] = V;
        }
        else {
            mem::Copy(Self.Data + Index, &V, sizeof(T));
        }

        Self.Size++;
        mem::Set(&V, 0, sizeof(T));
        return Self.Data[Index];
    }

    constexpr void GrowCapacity(this List& Self, USize NewSize) {
        if (Self.Capacity == 0) {
            Self.Data = mem::CountOf<T>(NewSize);
        }
        else {
            T* newData = mem::CountOf<T>(NewSize);
            mem::Copy(newData, Self.Data, Self.Capacity);
            mem::Deallocate(
                Cast<Address>(Self.Data),
                sizeof(T) * Self.Capacity
            );
        }
        Self.Capacity = NewSize;
    }

    constexpr void Clear(this List& Self) {
        Self.Deconstruct();
    }

    constexpr void Deconstruct(this List& Self) {
        for (auto& s : Self) {
            if constexpr (UseNew && !IsPrimitive) {
                s.Destroy();
            }
            else if constexpr (!IsPrimitive) {
                s.~T();
            }
        }
    }

    constexpr void Destroy(this List& Self) {
        Self.Deconstruct();

        mem::Deallocate(
            Cast<Address>(Self.Data), 
            sizeof(T) * Self.Capacity
        );
    }

    USize Capacity = 0;
    USize Size = 0;
    T* Data = nullptr;
};

