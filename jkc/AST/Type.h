#pragma once
#include "jkr/CoreHeader.h"

#include <string>

namespace AST {

struct [[nodiscard]] TypeDecl {
public:
    enum class Type {
        Unknown = 0,

        Void,
        Any,
        Byte,
        Int,
        UInt,
        Float,
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
    [[nodiscard]] constexpr bool IsAny() const { return Is(Type::Any) && Flags == 0; }
    [[nodiscard]] constexpr bool IsVoid() const { return Is(Type::Void) && Flags == 0; }
    [[nodiscard]] constexpr bool IsByte() const { return Is(Type::Byte) && Flags == 0; }
    [[nodiscard]] constexpr bool IsInt() const { return Is(Type::Int) && Flags == 0; }
    [[nodiscard]] constexpr bool IsUInt() const { return Is(Type::UInt) && Flags == 0; }
    [[nodiscard]] constexpr bool IsFloat() const { return Is(Type::Float) && Flags == 0; }
    [[nodiscard]] constexpr bool IsConstString() const { 
        return Is(Type::Byte) && (HasPtr() || HasArray()) && PointerDeep < 2; 
    }
    [[nodiscard]] constexpr bool IsArray() const { return !Is(Type::Unknown) && Flags & (Array); }

    [[nodiscard]] constexpr bool HasFlag(TypeFlags TF) const { return Flags & TF; }
    [[nodiscard]] constexpr bool HasConst() const { return HasFlag(Const); }
    [[nodiscard]] constexpr bool HasPtr() const { return HasFlag(Ptr); }
    [[nodiscard]] constexpr bool HasArray() const { return HasFlag(Array); }

    [[nodiscard]] constexpr bool IsNumeric() const { return IsByte() || IsInt() || IsUInt() || IsFloat(); }

    [[nodiscard]] constexpr bool operator==(const TypeDecl& RHS) const {
        return Primitive == RHS.Primitive
            && Flags == RHS.Flags
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
            for (Uint8 i = 0; i < PointerDeep; i++)
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
            str += "void";
            break;
        case Type::Any:
            str += "any";
            break;
        case Type::Byte:
            str += "byte";
            break;
        case Type::Int:
            str += "int";
            break;
        case Type::UInt:
            str += "uint";
            break;
        case Type::Float:
            str += "float";
            break;
        }

        return str;
    }

    Type Primitive = Type::Unknown;
    Uint8 Flags = None;
    Uint8 PointerDeep = 0;
    Uint32 ArrayLen = 0;
    Uint32 SizeInBits = 0;
};

}