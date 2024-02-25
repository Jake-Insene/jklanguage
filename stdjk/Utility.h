#pragma once
#include "stdjk/CoreTypes.h"

inline constexpr Byte Const4Max = 0xF;
inline constexpr Byte ByteMax = 0xFF;
inline constexpr Int IntMax = 0xFFFF'FFFF'FFFF'FFFF;
inline constexpr UInt UIntMax = UInt(-1);
inline constexpr Float FloatMax = 1.7976931348623158e+308;

template<typename To, typename From>
constexpr To Cast(From Val) {
    return reinterpret_cast<To>(Val);
}

template<typename To, typename From>
constexpr To IntCast(From Val) {
    return To(Val);
}
