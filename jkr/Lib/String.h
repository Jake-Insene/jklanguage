#pragma once
#include "jkr/CoreHeader.h"
#include "jkr/Mem/Utility.h"
#include "jkr/Mem/Allocator.h"
#include <string>

template<USize N>
constexpr USize Strlen(Char(&/*S*/)[N]) {
    return N;
}

constexpr USize Strlen(Str S) {
    USize i = 0;
    while (*S++ != '\0') i++;
    return i;
}

constexpr Char* UnsignedToBuff(Char* RNext, UInt Val) {
    do {
        *--RNext = (Char)('0' + Val % 10);
        Val /= 10;
    } while (Val != 0);

    return RNext;
}

struct [[nodiscard]] String {

    static constexpr String New() {
        return String{
            .Capacity = 0,
            .Size = 0,
            .Data = nullptr,
        };
    }

    void Destroy(this String& Self) {
        if (Self.Data) {
            mem::Deallocate(Self.Data);
        }
    }

    static constexpr String FromStrWithSize(Str S, USize Size) {
        Size += 1; // null termination
        Char* allocatedBuff = Cast<Char*>(mem::Allocate(Size));
        mem::Copy(allocatedBuff, S, Size);

        return String{
            .Capacity = Size,
            .Size = Size,
            .Data = allocatedBuff,
        };
    }

    static constexpr String FromStr(Str S) {return FromStrWithSize(S, Strlen(S)); }

    static constexpr String FromIter(const Char* Begin, const Char* End) {
        USize size = 0;
        Char* allocatedBuff = nullptr;

        size = (End - Begin) + 1;
        allocatedBuff = Cast<Char*>(mem::Allocate(size));

        USize i = 0;
        for (; Begin != End; ++Begin) {
            allocatedBuff[i] = *Begin;
            i++;
        }

        allocatedBuff[size - 1] = 0;
        return String{
            .Capacity = size,
            .Size = size,
            .Data = allocatedBuff,
        };
    }

    static constexpr String FromInt(Int Val) {
        Char buff[21] = {};
        Char* buffEnd = buff + 21;
        Char* rNext = buffEnd;
        const UInt uVal = UInt(Val);

        if (Val < 0) {
            rNext = UnsignedToBuff(rNext, 0 - uVal);
            *--rNext = '-';
        }
        else {
            rNext = UnsignedToBuff(rNext, uVal);
        }

        return String::FromIter(rNext, buffEnd);
    }

    static constexpr String FromUInt(UInt Val) {
        Char buff[21] = {};
        Char* buffEnd = buff + 21;
        Char* rNext = buffEnd;

        rNext = UnsignedToBuff(buffEnd, Val);
        return String::FromIter(rNext, buffEnd);
    }


    USize Capacity;
    USize Size;
    Char* Data;
};

