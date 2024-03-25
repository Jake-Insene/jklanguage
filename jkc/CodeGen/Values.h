#pragma once
#include "jkc/AST/Type.h"
#include "jkc/AST/Enums.h"
#include "jkc/Lexer/Token.h"
#include <cassert>

namespace CodeGen {

struct Constant {
    AST::TypeDecl Type = {};
    union {
        Int Signed = 0;
        UInt Unsigned;
        Float Real;
        void* Array;
    };
    UInt32 Address = 0;
};

struct [[nodiscard]] Local {
    Str Name = u8"";
    AST::TypeDecl Type = {};
    union {
        Byte Index = 0;
        Byte Reg;
    };
    bool IsInitialized = false;
    bool IsRegister = false;
};

struct [[nodiscard]] Global {
    Str Name = u8"";
    AST::TypeDecl Type = {};
    UInt32 Index = 0;
    Constant Value;
};

enum class TmpType {
    Err,
    Register,
    Local,
    LocalReg,
    Global,
    Constant,
    ArrayExpr,
    ArrayRef,
};

struct [[nodiscard]] TmpValue {
    TmpType Ty;

    union {
        Byte Reg;
        Byte Local;
        UInt16 Global;
        Float Real;
        UInt Data;
    };

    UInt32 Index;
    AST::TypeDecl Type;
    AST::BinaryOperation LastOp;

    [[nodiscard]] constexpr bool IsErr() const { return Ty == TmpType::Err; }
    [[nodiscard]] constexpr bool IsRegister() const { return Ty == TmpType::Register; }
    [[nodiscard]] constexpr bool IsLocal() const { return Ty == TmpType::Local; }
    [[nodiscard]] constexpr bool IsLocalReg() const { return Ty == TmpType::LocalReg; }
    [[nodiscard]] constexpr bool IsGlobal() const { return Ty == TmpType::Global; }
    [[nodiscard]] constexpr bool IsConstant() const { return Ty == TmpType::Constant; }
    [[nodiscard]] constexpr bool IsArrayExpr() const { return Ty == TmpType::ArrayExpr; }
    [[nodiscard]] constexpr bool IsArrayRef() const { return Ty == TmpType::ArrayRef; }
    [[nodiscard]] constexpr bool IsFunctionLocal() const { return IsLocal() || IsLocalReg(); }
};

struct StringTmp {
    constexpr StringTmp(Str Data, UInt Size) :
        Data(Data), Size(Size) {}
    constexpr ~StringTmp() {}

    Str Data;
    USize Size;
    SourceLocation Location;
};

}
