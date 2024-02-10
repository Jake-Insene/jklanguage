#pragma once
#include "jkr/Lib/List.h"

struct MemoryBuffer {
    using Iterator = List<Byte>::Iterator;
    using ConstIterator = List<Byte>::ConstIterator;

    static constexpr MemoryBuffer New(USize InitialSize = 0) {
        return MemoryBuffer{
            .Buff = List<Byte>::New(InitialSize),
        };
    }

    [[nodiscard]] constexpr Iterator begin()  { return Buff.begin(); }
    [[nodiscard]] constexpr ConstIterator begin() const { return Buff.begin(); }

    [[nodiscard]] constexpr Iterator end() { return Buff.end(); }
    [[nodiscard]] constexpr ConstIterator end() const { return Buff.end(); }

    constexpr MemoryBuffer& operator<<(Byte Val) {
        Buff.Push() = Val;
        return *this;
    }
    
    constexpr MemoryBuffer& operator<<(Uint16 Val) {
        Buff.Push() = (Byte(Val));
        Buff.Push() = (Byte(Val >> 8));
        return *this;
    }

    constexpr MemoryBuffer& operator<<(Uint32 Val) {
        Buff.Push() = (Byte(Val));
        Buff.Push() = (Byte(Val >> 8));
        Buff.Push() = (Byte(Val >> 16));
        Buff.Push() = (Byte(Val >> 24));
        return *this;
    }
    
    constexpr MemoryBuffer& operator<<(Uint64 Val) {
        Buff.Push() = (Byte(Val));
        Buff.Push() = (Byte(Val >> 8));
        Buff.Push() = (Byte(Val >> 16));
        Buff.Push() = (Byte(Val >> 24));
        Buff.Push() = (Byte(Val >> 32));
        Buff.Push() = (Byte(Val >> 40));
        Buff.Push() = (Byte(Val >> 48));
        Buff.Push() = (Byte(Val >> 56));
        return *this;
    }

    MemoryBuffer& operator<<(const MemoryBuffer& RHS) {
        for (auto& b : RHS) {
            *this << b;
        }
    }

    constexpr void Write(Byte* Mem, UInt Count) {
        for (Uint64 i = 0; i < Count; i++)
            Buff.Push() = Mem[i];
    }

    constexpr void Clear() {
        Buff.Clear();
    }

    constexpr void Destroy() {}

    List<Byte> Buff;
};
