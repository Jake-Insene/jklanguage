#pragma once
#include "stdjk/CoreHeader.h"

#include <string>

namespace AST {

struct [[nodiscard]] TypeDecl {
public:
    enum class Type {
        Unknown = 0,

        Void,
        Byte,
        Int,
        UInt,
        Float,
        Any,
    };

    enum TypeFlags {
        None,
        Const = 0b0001,
        Ptr = 0b0010,
        Array = 0b0100,
    };

public:
    [[nodiscard]] static constexpr TypeDecl Void() {
        return TypeDecl{
            Type::Void,
            None,
            0,
            0,
            0
        };
    }

    [[nodiscard]] constexpr bool Is(Type PrimitiveType) const { return Primitive == PrimitiveType; }
    [[nodiscard]] constexpr bool IsUnknown() const { return Is(Type::Unknown) && Flags == 0; }
    [[nodiscard]] constexpr bool IsVoid() const { return Is(Type::Void) && Flags == 0; }
    [[nodiscard]] constexpr bool IsAny() const { return Is(Type::Any) && Flags == 0; }
    [[nodiscard]] constexpr bool IsByte() const { return Is(Type::Byte) && Flags == 0; }
    [[nodiscard]] constexpr bool IsInt() const { return Is(Type::Int) && Flags == 0; }
    [[nodiscard]] constexpr bool IsUInt() const { return Is(Type::UInt) && Flags == 0; }
    [[nodiscard]] constexpr bool IsFloat() const { return Is(Type::Float) && Flags == 0; }
    [[nodiscard]] constexpr bool IsConstString() const { 
        return Is(Type::Byte) && HasConst() && (HasPtr() || HasArray()) && PointerDeep < 2;
    }
    [[nodiscard]] constexpr bool IsArray() const { return !Is(Type::Unknown) && Flags & (Array); }

    [[nodiscard]] constexpr bool HasFlag(TypeFlags TF) const { return Flags & TF; }
    [[nodiscard]] constexpr bool HasConst() const { return HasFlag(Const); }
    [[nodiscard]] constexpr bool HasPtr() const { return HasFlag(Ptr); }
    [[nodiscard]] constexpr bool HasArray() const { return HasFlag(Array); }

    [[nodiscard]] constexpr bool IsNumeric() const { return IsByte() || IsInt() || IsUInt() || IsFloat(); }

    [[nodiscard]] constexpr bool operator==(const TypeDecl& RHS) const {
        if (IsConstString() && RHS.IsConstString()) return true;

        UInt8 flags = Flags;
        if (flags & Const) {
            flags &= 0b11111110;
        }

        UInt8 rFlags = Flags;
        if (rFlags & Const) {
            rFlags &= 0b11111110;
        }
        return Primitive == RHS.Primitive
            && flags == rFlags
            && PointerDeep == RHS.PointerDeep
            && ArrayLen == RHS.ArrayLen;
    }

    [[nodiscard]] constexpr std::string ToString() const {
        std::string str;

        if (Flags & Array) {
            str += "[";
            if (ArrayLen)
                str += std::to_string(ArrayLen);
            str += "]";
        }

        if (Flags & Ptr) {
            for (UInt8 i = 0; i < PointerDeep; i++)
                str += "*";
        }

        if (Flags & Const) {
            str += "const ";
        }

        switch (Primitive) {
        case Type::Unknown:
            str += "<InvalidType>";
            break;
        case Type::Void:
            str += "Void";
            break;
        case Type::Any:
            str += "Any";
            break;
        case Type::Byte:
            str += "Byte";
            break;
        case Type::Int:
            str += "Int";
            break;
        case Type::UInt:
            str += "UInt";
            break;
        case Type::Float:
            str += "Float";
            break;
        }

        return str;
    }

    Type Primitive = Type::Unknown;
    UInt8 Flags = None;
    UInt8 PointerDeep = 0;
    UInt32 ArrayLen = 0;
    UInt32 SizeInBits = 0;
};

}