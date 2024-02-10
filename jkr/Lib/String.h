#pragma once
#include "jkr/CoreHeader.h"

constexpr USize Strlen(Str S) {
    USize i = 0;
    while (S[i] != '\0') i++;
    return i;
}

using NumericBuff = Char[21];

struct [[nodiscard]] String {

    static String New();

    static String FromStr(Str S);

    static void IntToString(Int Val, NumericBuff& Buff);
    static void UIntToString(UInt Val, NumericBuff& Buff);

    static String FromBuffer(Char* Buff, USize Size);

    Char* Data;
};

