#pragma once

template <typename T, T Val>
struct IntegralConstant {
    static constexpr T Value = Val;

    using ValueType = T;
    using Type = IntegralConstant;
};

template <bool Val>
using BoolConstant = IntegralConstant<bool, Val>;

template <typename, typename>
inline constexpr bool TIsSameAsV = false; // determine whether arguments are the same type
template <typename T>
inline constexpr bool TIsSameAsV<T, T> = true;

template <typename T1, typename T2>
struct TIsSameAs : BoolConstant<TIsSameAsV<T1, T2>> {};

template <typename T, class... TArgs>
inline constexpr bool IsSameAs = (TIsSameAsV<T, TArgs> || ...);

template<typename T>
inline constexpr bool IsPrimitive = IsSameAs<
    T, Byte, Int, UInt, USize, ISize, IntPtr, Address,
    char, short, int, long long,
    unsigned short, unsigned int, unsigned long long
>;
