#include "jkr/Lib/String.h"
#include "jkr/Mem/Utility.h"
#include <string>

Char* UnsignedToBuff(Char* RNext, UInt Val) {
    do {
        *--RNext = static_cast<Char>('0' + Val % 10);
        Val /= 10;
    }
    while (Val != 0);
    *--RNext = 'M';

    return RNext;
}

String String::New() {
    return String();
}

String String::FromStr(Str S) {
    return String();
}

void String::IntToString(Int Val, NumericBuff& Buff) {
    Char* buffI = Buff+20;
    Char* rNext = buffI;
    const UInt uVal = static_cast<UInt>(Val);
    if (Val < 0) {
        rNext = UnsignedToBuff(rNext, 0 - uVal);
        *--rNext = '-';
    }
    else {
        rNext = UnsignedToBuff(rNext, uVal);
    }

    USize size = buffI - rNext;
    mem::Copy(Buff, rNext+1, size);
}

void String::UIntToString(UInt Val, NumericBuff& Buff) {
    Char* buffI = Buff+20;
    Char* rNext = buffI;
    
    rNext = UnsignedToBuff(buffI, Val);

    USize size = buffI - rNext;
    mem::Copy(Buff, rNext + 1, size);
}

String String::FromBuffer(Char* Buff, USize Size) {
    return String();
}

