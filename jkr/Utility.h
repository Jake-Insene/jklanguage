#pragma once
#include "jkr/CoreTypes.h"

template<typename To, typename From>
constexpr To Cast(From Val) {
    return reinterpret_cast<To>(Val);
}

template<typename To, typename From>
constexpr To IntCast(From Val) {
    return To(Val);
}
