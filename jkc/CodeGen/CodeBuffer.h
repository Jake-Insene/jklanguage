#pragma once
#include <jkr/Vector.h>

struct CodeBuffer {
    using Iterator = Vector<Byte>::iterator;
    using ConstIterator = Vector<Byte>::const_iterator;

    constexpr CodeBuffer() {}

    constexpr ~CodeBuffer() {}

    [[nodiscard]] constexpr Iterator begin()  { return Buff.begin(); }
    [[nodiscard]] constexpr ConstIterator begin() const { return Buff.begin(); }

    [[nodiscard]] constexpr Iterator end() { return Buff.end(); }
    [[nodiscard]] constexpr ConstIterator end() const { return Buff.end(); }

    constexpr CodeBuffer& operator<<(UInt8 Val) {
        Buff.emplace_back(Val);
        return *this;
    }

    constexpr CodeBuffer& operator<<(UInt16 Val) {
        Buff.emplace_back(Byte(Val));
        Buff.emplace_back(Byte(Val>>8));
        return *this;
    }

    constexpr CodeBuffer& operator<<(UInt32 Val) {
        Buff.emplace_back(Byte(Val));
        Buff.emplace_back(Byte(Val >> 8));
        Buff.emplace_back(Byte(Val >> 16));
        Buff.emplace_back(Byte(Val >> 24));
        return *this;
    }

    constexpr CodeBuffer& operator<<(UInt64 Val) {
        Buff.emplace_back(Byte(Val));
        Buff.emplace_back(Byte(Val >> 8));
        Buff.emplace_back(Byte(Val >> 16));
        Buff.emplace_back(Byte(Val >> 24));
        Buff.emplace_back(Byte(Val >> 32));
        Buff.emplace_back(Byte(Val >> 40));
        Buff.emplace_back(Byte(Val >> 48));
        return *this;
    }
    
    CodeBuffer& operator<<(const CodeBuffer& RHS) {
        for (auto& b : RHS) {
            *this << b;
        }
    }

    constexpr void Clear() {
        Buff.clear();
    }

    Vector<Byte> Buff;
};
