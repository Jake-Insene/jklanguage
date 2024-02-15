#pragma once
#include "jkc/AST/Program.h"
#include "jkc/AST/Type.h"
#include "jkc/AST/Enums.h"
#include "jkc/CodeGen/Assembler.h"
#include "jkc/Utility/Memory.h"
#include "jkc/Utility/File.h"
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
};

enum DebugLevel {
    DBG_NONE,
    DBG_NORMAL,
};

enum Optimization {
    OPTIMIZATION_NONE,
    OPTIMIZATION_RELEASE_FAST,
    OPTIMIZATION_RELEASE_SMALL,
};

struct EmitOptions {
    DebugLevel Debug;
    Optimization OptimizationLevel;
};

struct Constant {
    AST::TypeDecl Type = {};
    union {
        Int Signed = 0;
        UInt Unsigned;
        Float Real;
    };
};

struct [[nodiscard]] TmpValue {
    TmpType Ty = TmpType::Err;

    union {
        Byte Reg;
        codefile::LocalType Local;
        codefile::GlobalType Global;
        Float Real;
        UInt Data = 0;
    };
    AST::TypeDecl Type = {};
    AST::BinaryOperation LastOp = (AST::BinaryOperation)-1;

    [[nodiscard]] constexpr bool IsErr() const { return Ty == TmpType::Err; }
    [[nodiscard]] constexpr bool IsRegister() const { return Ty == TmpType::Register; }
    [[nodiscard]] constexpr bool IsLocal() const { return Ty == TmpType::Local; }
    [[nodiscard]] constexpr bool IsLocalReg() const { return Ty == TmpType::LocalReg; }
    [[nodiscard]] constexpr bool IsGlobal() const { return Ty == TmpType::Global; }
    [[nodiscard]] constexpr bool IsConstant() const { return Ty == TmpType::Constant; }
};

struct Emitter {

    Emitter(StreamOutput& ErrorStream) :
        ErrorStream(ErrorStream) {}

    ~Emitter() {}

    void Emit(AST::Program& Program, FileType FileTy, EmitOptions Options, StreamOutput& Output);

    void Warn(const SourceLocation& Location, Str Format, ...);
    void Error(const SourceLocation& Location, Str Format, ...);
    void TypeError(const AST::TypeDecl& LHS, const AST::TypeDecl& RHS,
                   const SourceLocation& Location, Str Format, ...);
    void GlobalError(Str Format, ...);
    
    void PreDeclareStatement(mem::Ptr<AST::Statement>& Stat);
    void PreDeclareFunction(AST::Function* Fn);
    void PreDeclareVar(AST::Var* Var);
    void PreDeclareConstVal(AST::ConstVal* ConstVal);

    void EmitStatement(mem::Ptr<AST::Statement>& Stat);
    void EmitFunction(AST::Function* ASTFn);
    
    void EmitFunctionStatement(mem::Ptr<AST::Statement>& Stat, Function& Fn);
    void EmitFunctionReturn(AST::Return* Ret, Function& Fn);
    void EmitFunctionLocal(AST::Var* Var, Function& Fn);
    void EmitFunctionConstVal(AST::ConstVal* ConstVal, Function& Fn);
    void EmitFunctionIf(AST::If* ConstVal, Function& Fn);

    TmpValue GetID(const std::u8string& ID, Function& Fn, const SourceLocation& Location);
    Function* GetFn(mem::Ptr<AST::Expresion>& Target);

    TmpValue EmitFunctionExpresion(mem::Ptr<AST::Expresion>& Expr, Function& Fn);
    TmpValue EmitFunctionConstant(AST::Constant* Constant, Function& Fn);
    TmpValue EmitFunctionIdentifier(AST::Identifier* ID, Function& Fn);
    TmpValue EmitFunctionCall(AST::Call* Call, Function& Fn);
    
    TmpValue EmitFunctionBinaryOp(AST::BinaryOp* BinOp, Function& Fn);
    TmpValue EmitBinaryOp(TmpValue& Left, TmpValue& Right, AST::BinaryOperation Op, Function& Fn);

    TmpValue EmitFunctionUnary(AST::Unary* Unary, Function& Fn);
    TmpValue EmitFunctionDot(AST::Dot* Dot, Function& Fn);
    TmpValue EmitFunctionArrayList(AST::ArrayList* ArrayList, Function& Fn);
    TmpValue EmitFunctionBlock(AST::Block* Block, Function& Fn);

    void PushTmp(Function& Fn, const TmpValue& Tmp);
    void MoveTmp(Function& Fn, Uint8 Reg, const TmpValue& Tmp);

    Uint8 AllocateRegister() {
        for (auto& r : Registers) {
            if (!r.IsAllocated) {
                r.IsAllocated = true;
                return r.Index;
            }
        }
        assert("Register allocation fail");
        return Uint8(-1);
    }

    void DeallocateRegister(Uint8 Index) {
        assert(Index <= 15 && "Invalid register index");
        Registers[Index].IsAllocated = false;
    }

    StreamOutput& ErrorStream;
    bool Success = true;
    EmitOptions CurrentOptions;

    Assembler CodeAssembler;

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

    SymbolTable<Function> Functions;
    SymbolTable<Global> Globals;
    SymbolTable<Constant> Constants;
};

}
