#pragma once
#include "stdjk/Mem/Allocator.h"
#include "stdjk/Mem/Utility.h"
#include "stdjk/Debug.h"
#include "stdjk/Error.h"
#include "stdjk/Complt.h"

template<typename T, bool UseNew = true>
struct [[nodiscard]] List {

    using Type = T;
    using ElementType = RemovePointer<RemoveReference<T>>;
    using PointerType = AddPointer<ElementType>;
    using Iterator = Type*;
    using ConstIterator = const Type*;

    static constexpr List New(USize InitialSize) {
        return List{
            .Capacity = InitialSize,
            .Size = 0,
            .Data = mem::CountOf<Type>(InitialSize),
        };
    }

    [[nodiscard]] constexpr Iterator begin() { return Data; }
    [[nodiscard]] constexpr ConstIterator begin() const { return Data; }
    [[nodiscard]] constexpr Iterator end() { return &Data[Size]; }
    [[nodiscard]] constexpr ConstIterator end() const { return &Data[Size]; }

    [[nodiscard]] constexpr Type& operator[](this List& Self, USize Index) {
        RuntimeError(Index < Self.Size, STR("Index out of range"));
        return Self.Data[Index];
    }

    [[nodiscard]] constexpr const Type& operator[](this const List& Self, USize Index) {
        RuntimeError(Index < Self.Size, STR("Index out of range"));
        return Self.Data[Index];
    }

    [[nodiscard]] constexpr Type& Get(this List& Self, USize Index) {
        RuntimeError(Index < Self.Size, STR("Index out of range"));
        return Self.Data[Index];
    }

    template<typename... TArgs>
    constexpr Type& Push(this List& Self, TArgs&&... Args) {
        if (Self.Size >= Self.Capacity)
            Self.GrowCapacity(Self.Capacity ? Self.Capacity + (Self.Capacity / 2) : 2);

        if constexpr (UseNew && !IsPrimitive<ElementType> && !IsPointer<Type>) {
            return Self.Data[Self.Size++] = ElementType::New(Args...);
        }
        else if constexpr (IsPrimitive<ElementType> && !IsPointer<Type>) {
            return Self.Data[Self.Size++] = ElementType(Args...);
        }
        else {
            return Self.Data[Self.Size++];
        }
    }

    constexpr Type& Insert(this List& Self, USize Index, Type V) {
        if (Self.Size >= Self.Capacity)
            Self.GrowCapacity(Self.Capacity ? Self.Capacity + (Self.Capacity / 2) : 2);

        mem::Move(Self.Data + Index + 1, Self.Data + Index, sizeof(Type) * Self.Size);
        if constexpr (UseNew || IsPrimitive<Type>) {
            Self.Data[Index] = V;
        }
        else {
            mem::Copy(Self.Data + Index, &V, sizeof(Type));
        }

        Self.Size++;
        mem::Set(&V, 0, sizeof(Type));
        return Self.Data[Index];
    }

    constexpr void GrowCapacity(this List& Self, USize NewSize) {
        if (Self.Capacity == 0) {
            Self.Data = mem::CountOf<Type>(NewSize);
        }
        else {
            Type* newData = mem::CountOf<Type>(NewSize);
            mem::Copy(newData, Self.Data, sizeof(Type) * Self.Capacity);
            mem::Deallocate(
                Self.Data
            );
            Self.Data = newData;
        }
        Self.Capacity = NewSize;
    }

    constexpr bool Contains(this List& Self, Type Value) {
        if constexpr (IsPrimitive<Type>) {
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
            if constexpr (UseNew && !IsPrimitive<Type>) {
                for (auto& s : Self) {
                    mem::Destroy(s);
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
    Type* Data = nullptr;
};

