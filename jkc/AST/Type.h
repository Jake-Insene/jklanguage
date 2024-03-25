#pragma once
#include "jkr/CoreTypes.h"
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
        Array = 0b0100,
    };

public:
    [[nodiscard]] static constexpr TypeDecl Void() {
        return TypeDecl{
            Type::Void,
            None,
            0,
            0
        };
    }

    [[nodiscard]] static constexpr TypeDecl Int() {
        return TypeDecl{
            Type::Int,
            None,
            0,
            0
        };
    }

    [[nodiscard]] static constexpr TypeDecl UInt() {
        return TypeDecl{
            Type::UInt,
            None,
            0,
            0
        };
    }

    [[nodiscard]] static constexpr TypeDecl Float() {
        return TypeDecl{
            Type::Float,
            None,
            0,
            0
        };
    }

    constexpr TypeDecl(Type Primitive, UInt32 SizeInBits, UInt8 Flags, UInt32 ArrayLen) :
        Primitive(Primitive), SizeInBits(SizeInBits), Flags(Flags), ArrayLen(ArrayLen)
    {}

    constexpr TypeDecl() : Primitive(Type::Unknown), SizeInBits(0), Flags(0), ArrayLen(0) {}

    [[nodiscard]] constexpr bool Is(Type PrimitiveType) const { return Primitive == PrimitiveType; }
    [[nodiscard]] constexpr bool IsUnknown() const { return Is(Type::Unknown) && !HasArray(); }
    [[nodiscard]] constexpr bool IsVoid() const { return Is(Type::Void) && !HasArray(); }
    [[nodiscard]] constexpr bool IsAny() const { return Is(Type::Any) && !HasArray(); }
    [[nodiscard]] constexpr bool IsByte() const { return Is(Type::Byte) && !HasArray(); }
    [[nodiscard]] constexpr bool IsInt() const { return Is(Type::Int) && !HasArray(); }
    [[nodiscard]] constexpr bool IsUInt() const { return Is(Type::UInt) && !HasArray(); }
    [[nodiscard]] constexpr bool IsFloat() const { return Is(Type::Float) && !HasArray(); }
    [[nodiscard]] constexpr bool IsConstString() const { 
        return Is(Type::Byte) && HasConst() && (HasArray());
    }
    [[nodiscard]] constexpr bool IsArray() const { return !Is(Type::Unknown) && Flags & (Array); }

    [[nodiscard]] constexpr bool HasFlag(TypeFlags TF) const { return Flags & TF; }
    [[nodiscard]] constexpr bool HasConst() const { return HasFlag(Const); }
    [[nodiscard]] constexpr bool HasArray() const { return HasFlag(Array); }

    [[nodiscard]] constexpr bool IsNumeric() const { return IsByte() || IsInt() || IsUInt() || IsFloat(); }

    [[nodiscard]] constexpr bool operator==(const TypeDecl& RHS) const {
        if (IsConstString() && RHS.IsConstString()) return true;

        UInt8 flags = Flags;
        if (flags & Const) {
            flags &= 0b11111110;
        }

        UInt8 rFlags = RHS.Flags;
        if (rFlags & Const) {
            rFlags &= 0b11111110;
        }
        return Primitive == RHS.Primitive
            && flags == rFlags
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

    Type Primitive;
    UInt32 SizeInBits;
    UInt8 Flags;
    UInt32 ArrayLen;
};

}