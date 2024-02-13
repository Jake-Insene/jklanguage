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
    Global,
    Constant,
};

struct [[nodiscard]] TmpValue {
    TmpType Ty;

    union {
        Byte Reg;
        codefile::LocalType Local;
        codefile::GlobalType Global;
        Float Real;
        UInt Data;
    };
    AST::TypeDecl Type;
    AST::BinaryOperation LastOp;

    [[nodiscard]] constexpr bool IsErr() const { return Ty == TmpType::Err; }
    [[nodiscard]] constexpr bool IsRegister() const { return Ty == TmpType::Register; }
    [[nodiscard]] constexpr bool IsLocal() const { return Ty == TmpType::Local; }
    [[nodiscard]] constexpr bool IsGlobal() const { return Ty == TmpType::Global; }
    [[nodiscard]] constexpr bool IsConstant() const { return Ty == TmpType::Constant; }
};

struct Emitter {

    Emitter(StreamOutput& ErrorStream) :
        ErrorStream(ErrorStream) {}

    ~Emitter() {}

    void Emit(AST::Program& Program, FileType FileTy, StreamOutput& Output);

    void Error(Str FileName, USize Line, Str Format, ...);

    void TypeError(const AST::TypeDecl& LHS, const AST::TypeDecl& RHS, Str FileName, USize Line);

    void GlobalError(Str Format, ...);
    
    void PreDeclareStatement(mem::Ptr<AST::Statement>& Stat);
    void PreDeclareFunction(AST::Function* Fn);
    void PreDeclareVar(AST::Var* Var);
    void PreDeclareConstVal(AST::ConstVal* ConstVal);

    void EmitStatement(mem::Ptr<AST::Statement>& Stat);
    void EmitFunction(AST::Function* ASTFn);
    
    void EmitFunctionStatement(mem::Ptr<AST::Statement>& Stat, JKFunction& Fn);
    void EmitFunctionReturn(AST::Return* Ret, JKFunction& Fn);
    void EmitFunctionLocal(AST::Var* Var, JKFunction& Fn);
    void EmitFunctionConstVal(AST::ConstVal* ConstVal, JKFunction& Fn);
    void EmitFunctionIf(AST::If* ConstVal, JKFunction& Fn);

    TmpValue GetID(const std::u8string& ID, JKFunction& Fn, Str FileName, USize Line);
    JKFunction* GetFn(mem::Ptr<AST::Expresion>& Target);

    TmpValue EmitFunctionExpresion(mem::Ptr<AST::Expresion>& Expr, JKFunction& Fn);
    TmpValue EmitFunctionConstant(AST::Constant* Constant, JKFunction& Fn);
    TmpValue EmitFunctionIdentifier(AST::Identifier* ID, JKFunction& Fn);
    TmpValue EmitFunctionCall(AST::Call* Call, JKFunction& Fn);
    TmpValue EmitFunctionBinaryOp(AST::BinaryOp* BinOp, JKFunction& Fn);
    TmpValue EmitFunctionUnary(AST::Unary* Unary, JKFunction& Fn);
    TmpValue EmitFunctionDot(AST::Dot* Dot, JKFunction& Fn);
    TmpValue EmitFunctionArrayList(AST::ArrayList* ArrayList, JKFunction& Fn);
    TmpValue EmitFunctionBlock(AST::Block* Block, JKFunction& Fn);

    void PushTmp(JKFunction& Fn, const TmpValue& Tmp);
    void MoveTmp(JKFunction& Fn, Uint8 Reg, const TmpValue& Tmp);

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

    JKRAssembler Assembler;

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

    SymbolTable<JKFunction> Functions;
    SymbolTable<JKGlobal> Globals;
};

}
