#pragma once
#include "jkc/AST/Program.h"
#include "jkc/AST/Type.h"
#include "jkc/AST/Enums.h"
#include "jkc/CodeGen/Assembler.h"
#include "jkc/Utility/Memory.h"
#include "jkc/Utility/File.h"
#include "stdjk/String.h"
#include <unordered_map>
#include <assert.h>

namespace CodeGen {

enum class FileType {
    Executable,
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

enum DebugLevel {
    DBG_NONE,
    DBG_NORMAL,
};

enum Optimization {
    OPTIMIZATION_NONE,
    OPTIMIZATION_RELEASE_FAST,
};

struct EmitOptions {
    DebugLevel Debug;
    Optimization OptimizationLevel;
};

struct RegisterInfo {
    Byte Index = 0;
    bool IsAllocated = false;
};

struct [[nodiscard]] TmpValue {
    TmpType Ty = TmpType::Err;

    union {
        Byte Reg;
        Byte Local;
        UInt32 Global;
        Float Real;
        UInt Data = 0;
    };
    AST::TypeDecl Type = {};
    AST::BinaryOperation LastOp = AST::BinaryOperation::None;

    [[nodiscard]] constexpr bool IsErr() const { return Ty == TmpType::Err; }
    [[nodiscard]] constexpr bool IsRegister() const { return Ty == TmpType::Register; }
    [[nodiscard]] constexpr bool IsLocal() const { return Ty == TmpType::Local; }
    [[nodiscard]] constexpr bool IsLocalReg() const { return Ty == TmpType::LocalReg; }
    [[nodiscard]] constexpr bool IsGlobal() const { return Ty == TmpType::Global; }
    [[nodiscard]] constexpr bool IsConstant() const { return Ty == TmpType::Constant; }
    [[nodiscard]] constexpr bool IsArrayExpr() const { return Ty == TmpType::ArrayExpr; }
    [[nodiscard]] constexpr bool IsArrayRef() const { return Ty == TmpType::ArrayRef; }
};

struct Emitter {

    Emitter(StreamOutput& ErrorStream) :
        ErrorStream(ErrorStream), Context(), CurrentOptions() {}

    ~Emitter() {}

    void Emit(AST::Program& Program, FileType FileTy, EmitOptions Options, StreamOutput& Output);

    void Warn(const SourceLocation& Location, Str Format, ...);
    void Error(const SourceLocation& Location, Str Format, ...);
    void TypeError(const AST::TypeDecl& LHS, const AST::TypeDecl& RHS,
                   const SourceLocation& Location, Str Format, ...);
    void GlobalError(Str Format, ...);
    
    void PreDeclareStatement(AST::Statement* Stat);
    void PreDeclareFunction(AST::Function* ASTFn);
    void PreDeclareVar(AST::Var* Var);
    void PreDeclareConstVal(AST::ConstVal* ConstVal);

    TmpValue EmitExpresion(AST::Expresion* Expr);
    void EmitStatement(AST::Statement* Stat);
    void EmitFunction(AST::Function* ASTFn);
    void EmitVar(AST::Var* Var);
    void EmitConstVal(AST::ConstVal* ConstVal);
    
    void EmitFunctionStatement(AST::Statement* Stat, Function& Fn);
    void EmitFunctionReturn(AST::Return* Ret, Function& Fn);
    void EmitFunctionLocal(AST::Var* Var, Function& Fn);
    void EmitFunctionConstVal(AST::ConstVal* ConstVal, Function& Fn);
    void EmitFunctionIf(AST::If* ConstVal, Function& Fn);

    TmpValue GetID(const std::u8string& ID, Function& Fn, const SourceLocation& Location);
    Function* GetFn(AST::Expresion* Target);

    TmpValue EmitFunctionExpresion(AST::Expresion* Expr, Function& Fn);
    TmpValue EmitFunctionConstant(AST::Constant* Constant, Function& Fn);
    TmpValue EmitFunctionIdentifier(AST::Identifier* ID, Function& Fn);
    TmpValue EmitFunctionCall(AST::Call* Call, Function& Fn);
    
    TmpValue EmitFunctionBinaryOp(AST::BinaryOp* BinOp, Function& Fn);
    TmpValue EmitBinaryOp(TmpValue& Left, TmpValue& Right, AST::BinaryOperation Op, Function& Fn);

    TmpValue EmitFunctionUnary(AST::Unary* Unary, Function& Fn);
    TmpValue EmitFunctionDot(AST::Dot* Dot, Function& Fn);
    TmpValue EmitFunctionArrayList(AST::ArrayList* ArrayList, Function& Fn);
    TmpValue EmitFunctionBlock(AST::Block* Block, Function& Fn);
    TmpValue EmitFunctionArrayAccess(AST::ArrayAccess* ArrayAccess, Function& Fn);

    void PushTmp(Function& Fn, const TmpValue& Tmp);
    void MoveTmp(Function& Fn, UInt8 Reg, const TmpValue& Tmp);
    codefile::PrimitiveType TypeToPrimitive(const AST::TypeDecl& Type);
    void MoveConst(Function& Fn, Byte Dest, UInt64 Const);

    UInt8 AllocateRegister() {
        for (auto& r : Registers) {
            if (!r.IsAllocated) {
                r.IsAllocated = true;
                return r.Index;
            }
        }
        assert("Register allocation fail");
        return UInt8(-1);
    }

    void DeallocateRegister(UInt8 Index) {
        assert(Index <= 15 && "Invalid register index");
        Registers[Index].IsAllocated = false;
    }

    StreamOutput& ErrorStream;
    bool Success = true;
    EmitOptions CurrentOptions;

    Assembler CodeAssembler;
    struct {
        bool IsInReturn;
        bool IsInCall;
        bool IsLast;
        bool IsInIf;
        bool IsInElse;
    } Context;

    RegisterInfo Registers[15] = {
        {0, false},
        {1, false},
        {2, false},
        {3, false},
        {4, false},
        {5, false},
        {6, false},
        {7, false},
        {8, false },
        {9, false },
        {10, false },
        {11, false },
        {12, false },
        {13, false },
        {14, false },
    };

    struct StringTmp {
        static StringTmp New(Str Data, UInt Size) {
            return StringTmp{Data, Size};
        }
        void Destroy(this StringTmp& /*Self*/) {}

        Str Data;
        USize Size;
    };

    SymbolTable<Function> Functions;
    SymbolTable<Global> Globals;
    SymbolTable<Constant> Constants;
    std::unordered_map<std::u8string, UInt32> NativeLibraries;
    List<StringTmp> Strings;
};

}