#pragma once
#include "stdjk/Mem/Allocator.h"
#include "stdjk/Mem/Utility.h"
#include "stdjk/Debug.h"
#include "stdjk/Error.h"
#include "stdjk/Complt.h"

template<typename T, bool UseNew = true>
struct [[nodiscard]] List {

    using Iterator = T*;
    using ConstIterator = const T*;

    static constexpr List New(USize InitialSize) {
        if (InitialSize == 0) {
            InitialSize = 4;
        }
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
        RuntimeError(Index < Self.Size, STR("Index out of range"));
        return Self.Data[Index];
    }

    [[nodiscard]] constexpr const T& operator[](this const List& Self, USize Index) {
        RuntimeError(Index < Self.Size, STR("Index out of range"));
        return Self.Data[Index];
    }

    [[nodiscard]] constexpr T& Get(this List& Self, USize Index) {
        RuntimeError(Index < Self.Size, STR("Index out of range"));
        return Self.Data[Index];
    }

    template<typename... TArgs>
    constexpr T& Push(this List& Self, TArgs&&... Args) {
        if (Self.Size >= Self.Capacity)
            Self.GrowCapacity(Self.Capacity ? Self.Capacity + (Self.Capacity/2) : 2);

        if constexpr (UseNew && !IsPrimitive<T>) {
            return Self.Data[Self.Size++] = T::New(Args...);
        }
        else if constexpr (IsPrimitive<T>) {
            return Self.Data[Self.Size++] = T(Args...);
        }
        else {
            return Self.Data[Self.Size++];
        }
    }

    constexpr T& Insert(this List& Self, USize Index, T V) {
        if (Self.Size >= Self.Capacity)
            Self.GrowCapacity(Self.Capacity ? Self.Capacity + (Self.Capacity / 2) : 2);

        mem::Move(Self.Data + Index + 1, Self.Data + Index, sizeof(T) * Self.Size);
        if constexpr (UseNew || IsPrimitive<T>) {
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
            mem::Copy(newData, Self.Data, sizeof(T) * Self.Capacity);
            mem::Deallocate(
                Self.Data
            );
            Self.Data = newData;
        }
        Self.Capacity = NewSize;
    }

    constexpr bool Contains(this List& Self, T Value) {
        if constexpr (IsPrimitive<T>) {
            for (auto& e : Self) {
                if (e == Value)
                    return true;
            }
        }

        return false;
    }

    constexpr void Clear(this List& Self) {
        Self.Deconstruct();

        Self.Size = 0;
    }

    constexpr void Deconstruct(this List& Self) {
        if (Self.Size) {
            if constexpr (UseNew && !IsPrimitive<T>) {
                for (auto& s : Self) {
                    s.Destroy();
                }
            }
        }
    }

    constexpr void Destroy(this List& Self) {
        Self.Deconstruct();

        if (Self.Capacity) {
            mem::Deallocate(
                Self.Data
            );
        }
    }

    USize Capacity = 0;
    USize Size = 0;
    T* Data = nullptr;
};

