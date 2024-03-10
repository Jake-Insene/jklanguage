#pragma once
#include <utility>

// Utils

template <typename T, T Val>
struct IntegralConstant {
    static constexpr T Value = Val;

    using ValueType = T;
    using Type = IntegralConstant;
};

template <bool Val>
using BoolConstant = IntegralConstant<bool, Val>;

template<typename... TArgs>
using VoidT = void;

// SameAs

template <typename, typename>
inline constexpr bool IsSameV = false;
template <typename T>
inline constexpr bool IsSameV<T, T> = true;

template <typename T1, typename T2>
struct TIsSameAs : BoolConstant<IsSameV<T1, T2>> {};

template <typename T, typename... TArgs>
inline constexpr bool IsSameAs = (IsSameV<T, TArgs> || ...);

template<typename T>
inline constexpr bool IsPrimitive = IsSameAs<
    T, Byte, Char, Int, UInt, USize, ISize, IntPtr, Address,
    char, short, int, long long,
    unsigned short, unsigned int, unsigned long long
>;

// Add pointer

template <typename>
constexpr bool IsPointerV  = false;

template <typename T>
constexpr bool IsPointerV <T*> = true;

template <typename T>
constexpr bool IsPointerV <T* const> = true;

template <typename T>
constexpr bool IsPointerV <T* volatile> = true;

template <typename T>
constexpr bool IsPointerV <T* const volatile> = true;

template<typename T>
inline constexpr bool IsPointer = BoolConstant<IsPointerV<T>>::Value;

// Remove reference

template <typename T>
struct TRemoveReference {
    using Type = T;
    using ConstRefType = const T;
};

template <typename T>
struct TRemoveReference<T&> {
    using Type = T;
    using ConstRefType = const T&;
};

template <typename T>
struct TRemoveReference<T&&> {
    using Type = T;
    using ConstRefType = const T&&;
};

template <typename T>
using RemoveReference = typename TRemoveReference<T>::Type;

// Remove pointer

template <typename T>
struct TRemovePointer {
    using Type = T;
};

template <typename T>
struct TRemovePointer<T*> {
    using Type = T;
};

template <typename T>
struct TRemovePointer<T* const> {
    using Type = T;
};

template <typename T>
struct TRemovePointer<T* volatile> {
    using Type = T;
};

template <typename T>
struct TRemovePointer<T* const volatile> {
    using Type = T;
};

template <typename T>
using RemovePointer = typename TRemovePointer<T>::Type;

// Add pointer

template <typename T, typename = void>
struct TAddPointer {
    using Type = T;
};

template <typename T>
struct TAddPointer<T, VoidT<RemoveReference<T>*>> { // (pointer Type can be formed)
    using Type = RemoveReference<T>*;
};

template <typename T>
using AddPointer = typename TAddPointer<T>::Type;
