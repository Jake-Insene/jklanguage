#include "jkc/CodeGen/Emitter.h"
#include "jkc/AST/Statements.h"
#include "jkc/AST/Expresions.h"
#include <jkr/CodeFile/Header.h>
#include <jkr/CodeFile/Function.h>

#define MATH_NOT_EQUAL(RESULT, LEFT, RIGHT, TYPE) {\
    RESULT.Ty = TmpType::Register;\
    Uint8 rightReg = 0;\
    if(!RIGHT.IsRegister()) {\
        rightReg = AllocateRegister();\
        MoveTmp(Fn, rightReg, RIGHT);\
    }\
    else {\
        rightReg = RIGHT.Reg;\
    }\
    if(LEFT.IsRegister()) {\
        RESULT.Reg = LEFT.Reg;\
        if (LEFT.Type.IsUInt()) {                              \
            Assembler.##TYPE(Fn, LEFT.Reg, LEFT.Reg, rightReg);    \
        }                                                       \
        else if (LEFT.Type.IsInt()) {                          \
            Assembler.I##TYPE(Fn, LEFT.Reg, LEFT.Reg, rightReg);   \
        }                                                       \
        else if (LEFT.Type.IsFloat()) {                        \
            Assembler.F##TYPE(Fn, LEFT.Reg, LEFT.Reg, rightReg);   \
        }\
    }\
    if(LEFT.IsLocal()) {\
        Uint8 res = AllocateRegister();\
        RESULT.Reg = res;\
        MoveTmp(Fn, res, LEFT);\
        if (LEFT.Type.IsUInt()) {                              \
            Assembler.##TYPE(Fn, res, res, rightReg);    \
        }                                                       \
        else if (LEFT.Type.IsInt()) {                          \
            Assembler.I##TYPE(Fn, res, res, rightReg);   \
        }                                                       \
        else if (LEFT.Type.IsFloat()) {                        \
            Assembler.F##TYPE(Fn, res, res, rightReg);   \
        }\
    }\
    DeallocateRegister(rightReg);\
}

#define MATH_EQUAL(LEFT, RIGHT, TYPE) {\
    Uint8 rightReg = 0;\
    if(!RIGHT.IsRegister()) {\
        rightReg = AllocateRegister();\
        MoveTmp(Fn, rightReg, RIGHT);\
    }\
    else {\
        rightReg = RIGHT.Reg;\
    }\
    if(LEFT.IsRegister()) {\
        if (LEFT.Type.IsUInt()) {                              \
            Assembler.##TYPE(Fn, LEFT.Reg, LEFT.Reg, rightReg);    \
        }                                                       \
        else if (LEFT.Type.IsInt()) {                          \
            Assembler.I##TYPE(Fn, LEFT.Reg, LEFT.Reg, rightReg);   \
        }                                                       \
        else if (LEFT.Type.IsFloat()) {                        \
            Assembler.F##TYPE(Fn, LEFT.Reg, LEFT.Reg, rightReg);   \
        }\
    }\
    if(LEFT.IsLocal()) {\
        Uint8 tmp = AllocateRegister();\
        MoveTmp(Fn, tmp, LEFT);        \
        if (LEFT.Type.IsUInt()) {                              \
            Assembler.##TYPE(Fn, tmp, tmp, rightReg);    \
        }                                                       \
        else if (LEFT.Type.IsInt()) {                          \
            Assembler.I##TYPE(Fn, tmp, tmp, rightReg);   \
        }                                                       \
        else if (LEFT.Type.IsFloat()) {                        \
            Assembler.F##TYPE(Fn, tmp, tmp, rightReg);   \
        }\
        DeallocateRegister(tmp);       \
    }\
    DeallocateRegister(rightReg);\
}

namespace CodeGen {

void Emitter::Emit(AST::Program& Program, FileType FileTy, StreamOutput& Output) {
    Functions.Clear();
    Globals.Clear();

    for (auto& statement : Program.Statements) {
        PreDeclareStatement(statement);
    }

    for (auto& stat : Program.Statements) {
        EmitStatement(stat);
    }

    codefile::FileHeader header = {
            .Signature = {},
            .CheckSize = sizeof(codefile::FileHeader),
    };

    mem::Copy(header.Signature, codefile::Signature, 8);
    header.CountOfFunctions = (Uint32)Functions.Size();
    header.CountOfGlobals = (Uint32)Globals.Size();

    if (FileTy == FileType::Executable) {
        auto it = Functions.Find(STR("Main"));
        if (it == Functions.end()) {
            GlobalError(STR("The entry point was not defined"));
            return;
        }
        
        header.Attributes |= codefile::AttributeExecutable;
        header.EntryPoint = (Uint32)it->second;
        header.MajorVersion = 1;
        header.MinorVersion = 0;
    }

    // Calculate CheckSize
    for (auto& fn : Functions.Data) {
        header.CheckSize += Uint32(sizeof(codefile::FunctionHeader) + fn.Code.Buff.Size);
    }

    // Writing
    Output.Write((Byte*)&header, sizeof(codefile::FileHeader));
    for (auto& fn : Functions.Data) {
        codefile::FunctionHeader fnHeader = {
            .Attributes = 0,
            .Arguments = fn.CountOfArguments,
            .LocalCount = (codefile::LocalType)fn.Locals.Size(),
            .SizeOfCode = (Uint32)fn.Code.Buff.Size,
        };

        Output.Write((Byte*)&fnHeader, sizeof(codefile::FunctionHeader));
        Output.Write((Byte*)fn.Code.Buff.Data, fn.Code.Buff.Size);
    }

    // End
    for (auto& fn : Functions.Data) {
        fn.Code.Destroy();
    }
}

void Emitter::Error(Str FileName, USize Line, Str Format, ...) {
    ErrorStream.Print(STR("{s}:{u}:\n\tError: "), FileName, Line);
    va_list args;
    va_start(args, Format);
    ErrorStream.PrintlnVa(Format, args);
    va_end(args);
    Success = false;
}

void Emitter::TypeError(const AST::TypeDecl& LHS, const AST::TypeDecl& RHS, Str FileName, USize Line) {
    if (LHS.IsNumeric() && RHS.IsNumeric()) {
        if ((LHS.IsFloat() && !RHS.IsFloat())
            || (RHS.IsFloat() && !LHS.IsFloat())) {
            Error(FileName, Line, STR("Possible data loss during conversion"));
        }
    }
    else {
        Error(FileName, Line,
            STR("Invalid conversion from '{s}' to '{s}'"),
            LHS.ToString().c_str(),
            RHS.ToString().c_str()
        );
    }
}

void Emitter::GlobalError(Str Format, ...) {
    ErrorStream.Print(STR("Error: "));

    va_list args;
    va_start(args, Format);
    ErrorStream.PrintlnVa(Format, args);
    va_end(args);
    Success = false;
}

void Emitter::PreDeclareStatement(mem::Ptr<AST::Statement>& Stat) {
    if (Stat->Type == AST::StatementType::Function) {
        PreDeclareFunction((AST::Function*)Stat.Data);
    }
    else if (Stat->Type == AST::StatementType::Var) {
        PreDeclareVar((AST::Var*)Stat.Data);
    }
    else if (Stat->Type == AST::StatementType::ConstVal) {
        PreDeclareConstVal((AST::ConstVal*)Stat.Data);
    }
}

void Emitter::PreDeclareFunction(AST::Function* Fn) {
    auto& fn = Functions.Emplace(Fn->Name);
    fn.Name = Fn->Name.data();

    for (auto& param : Fn->Parameters) {
        auto& local = fn.Locals.Emplace(param.Name);
        local.Name = param.Name.c_str();
        local.Type = param.Type;
        local.Index = (codefile::LocalType)fn.Locals.Size() - 1;
    }

    fn.Index = (Uint32)Functions.Size() - 1;
    fn.CountOfArguments = (Uint8)Fn->Parameters.Size;
    fn.Type = Fn->FunctionType;
}

void Emitter::PreDeclareVar(AST::Var* Var) {
}

void Emitter::PreDeclareConstVal(AST::ConstVal* ConstVal) {}


void Emitter::EmitStatement(mem::Ptr<AST::Statement>& Stat) {
    if (Stat->Type == AST::StatementType::Function) {
        EmitFunction((AST::Function*)Stat.Data);
    }
}

void Emitter::EmitFunction(AST::Function* ASTFn) {
    auto& fn = Functions.Get(Functions.Find(ASTFn->Name)->second);

    if (ASTFn->IsDefined) {
        for (auto& stat : ASTFn->Body->Statements) {
            EmitFunctionStatement(stat, fn);
        }
    }
}

void Emitter::EmitFunctionStatement(mem::Ptr<AST::Statement>& Stat, JKFunction& Fn) {
    if (Stat->Type == AST::StatementType::Return) {
        EmitFunctionReturn((AST::Return*)Stat.Data, Fn);
    }
    else if (Stat->Type == AST::StatementType::Var) {
        EmitFunctionLocal((AST::Var*)Stat.Data, Fn);
    }
    else if (Stat->Type == AST::StatementType::ConstVal) {
        EmitFunctionConstVal((AST::ConstVal*)Stat.Data, Fn);
    }
    else if (Stat->Type == AST::StatementType::If) {
        EmitFunctionIf((AST::If*)Stat.Data, Fn);
    }
}

void Emitter::EmitFunctionReturn(AST::Return* Ret, JKFunction& Fn) {
    if (Ret->Value) {
        TmpValue tmp = EmitFunctionExpresion(Ret->Value, Fn);
        if (tmp.IsErr()) return;
        TypeError(Fn.Type, tmp.Type, Ret->FileName, Ret->Line);

        if (tmp.IsRegister()) {
            Assembler.RRet(Fn, tmp.Reg);
            DeallocateRegister(tmp.Reg);
        }
        else if(tmp.IsLocal()) {
            Assembler.LRet(Fn, tmp.Local);
        }
        else if (tmp.IsGlobal()) {
            Assembler.GRet(Fn, tmp.Global);
        }
        else if (tmp.IsConstant()) {
            Uint8 reg = AllocateRegister();
            MoveTmp(Fn, reg, tmp);
            Assembler.RRet(Fn, reg);
            DeallocateRegister(reg);
        }

    }
    else {
        Assembler.Ret(Fn);
        TypeError(
            AST::TypeDecl::Void(),
            Fn.Type,
            Ret->FileName,
            Ret->Line
        );
    }
}

void Emitter::EmitFunctionLocal(AST::Var* Var, JKFunction& Fn) {
}

void Emitter::EmitFunctionConstVal(AST::ConstVal* ConstVal, JKFunction& Fn) {
}

void Emitter::EmitFunctionIf(AST::If* _If, JKFunction& Fn) {
    TmpValue result = EmitFunctionExpresion(_If->Expr, Fn);

    switch (result.LastOp) {
    case AST::BinaryOperation::Comparision:
        Assembler.Jmp(Fn, codefile::OpCode::Jne, 0);
        break;
    }
    USize toResolve = Fn.Code.Buff.Size - 4;

    (void)EmitFunctionExpresion(_If->Body, Fn);
    if (_If->Elif) {
        *((Uint32*)&Fn.Code.Buff[toResolve]) = (Uint32)Fn.Code.Buff.Size;
        EmitFunctionIf((AST::If*)_If->Elif.Data, Fn);
    }
    else if (_If->ElseBlock) {
        (void)EmitFunctionExpresion(_If->ElseBlock, Fn);
    }
    else {
        *((Uint32*)&Fn.Code.Buff[toResolve]) = (Uint32)Fn.Code.Buff.Size;
    }

}

TmpValue Emitter::GetID(const std::u8string& ID, JKFunction& Fn, Str FileName, USize Line) {
    {
        auto it = Fn.Locals.Find(ID);
        if (it != Fn.Locals.end()) {
            auto& local = Fn.Locals.Get(it->second);
            return TmpValue{
                .Ty = TmpType::Local,
                .Data = local.Index,
                .Type = local.Type,
            };
        }
    }

    {
        auto it = Globals.Find(ID);
        if (it == Globals.end()) {
            Error(FileName, Line,
                STR("Undefined reference to '{s}'"),
                ID.c_str()
            );
        }
        else {
            auto& global = Globals.Get(it->second);
            return TmpValue{
                .Ty = TmpType::Local,
                .Data = global.Index,
                .Type = global.Type,
            };
        }
    }

    return TmpValue();
}

JKFunction* Emitter::GetFn(mem::Ptr<AST::Expresion>& Target) {
    if (Target->Type == AST::ExpresionType::Identifier) {
        auto id = (AST::Identifier*)Target.Data;
        auto it = Functions.Find(id->ID);
        if (it != Functions.end()) {
            return &Functions.Get(it->second);
        }

        Error(id->FileName, id->Line,
            STR("Undefined reference to function '{s}'"),
            id->ID.c_str()
        );
    }
    return nullptr;
}

TmpValue Emitter::EmitFunctionExpresion(mem::Ptr<AST::Expresion>& Expr, JKFunction& Fn) {
    if (Expr->Type == AST::ExpresionType::Constant) {
        return EmitFunctionConstant((AST::Constant*)Expr.Data, Fn);
    }
    else if(Expr->Type == AST::ExpresionType::Identifier) {
        return EmitFunctionIdentifier((AST::Identifier*)Expr.Data, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::Group) {
        return EmitFunctionExpresion(((AST::Group*)Expr.Data)->Value, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::Call) {
        return EmitFunctionCall((AST::Call*)Expr.Data, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::BinaryOp) {
        return EmitFunctionBinaryOp((AST::BinaryOp*)Expr.Data, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::Unary) {
        return EmitFunctionUnary((AST::Unary*)Expr.Data, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::Dot) {
        return EmitFunctionDot((AST::Dot*)Expr.Data, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::ArrayList) {
        return EmitFunctionArrayList((AST::ArrayList*)Expr.Data, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::Block) {
        return EmitFunctionBlock((AST::Block*)Expr.Data, Fn);
    }

    assert(0 && "Invalid expresion");
    return TmpValue();
}

TmpValue Emitter::EmitFunctionConstant(AST::Constant* Constant, JKFunction& Fn) {
    TmpValue tmp = {};
    tmp.Ty = TmpType::Constant;
    tmp.Type = Constant->ValueType;
    tmp.Data = Constant->Unsigned;

    if (Constant->ValueType.IsUInt() || Constant->ValueType.IsInt()) {
        if (Constant->Unsigned <= UINT8_MAX) {
            tmp.Type.SizeInBits = 8;
        }
        else if (Constant->Unsigned <= UINT16_MAX) {
            tmp.Type.SizeInBits = 16;
        }
        else if (Constant->Unsigned <= UINT32_MAX) {
            tmp.Type.SizeInBits = 32;
        }
    }
    return tmp;
}

TmpValue Emitter::EmitFunctionIdentifier(AST::Identifier* ID, JKFunction& Fn) {
    return GetID(ID->ID, Fn, ID->FileName, ID->Line);
}

TmpValue Emitter::EmitFunctionCall(AST::Call* Call, JKFunction& Fn) {
    TmpValue result = {};
    JKFunction* target = GetFn(Call->Target);
    if (target == nullptr) return result;
    
    List<Byte> usedRegisters;
    for (auto& reg : Registers) {
        if (reg.IsAllocated) {
            usedRegisters.Push(reg.Index);
            Assembler.RPush(Fn, reg.Index);
            DeallocateRegister(reg.Index);
        }
    }

    if (target->CountOfArguments != Call->Arguments.Size) {
        Error(Call->FileName, Call->Line,
            STR("{u} arguments was expected but {u} was founded"),
            (UInt)target->CountOfArguments,
            Call->Arguments.Size
        );
    }

    result.Type = target->Type;
    for (Int32 i = target->CountOfArguments - 1; i >= 0; i--) {
        TmpValue arg = EmitFunctionExpresion(Call->Arguments[i], Fn);
        auto& local = target->Locals.Get(i);

        TypeError(local.Type, arg.Type, Call->FileName, Call->Line);

        PushTmp(Fn, arg);
    }

    Assembler.Call(Fn, target->Index);

    if(usedRegisters.Size)
    {
        auto end = usedRegisters.end();
        while (--end != usedRegisters.begin() - 1) {
            Assembler.RPop(Fn, *end);
            Registers[*end].IsAllocated = true;
        }
    }

    result.Ty = TmpType::Register;
    result.Data = (Uint64)AllocateRegister();
    Assembler.RMov(Fn, (Uint8)result.Data);

    usedRegisters.Destroy();
    return result;
}

TmpValue Emitter::EmitFunctionBinaryOp(AST::BinaryOp* BinOp, JKFunction& Fn) {
    TmpValue left = EmitFunctionExpresion(BinOp->Left, Fn);
    TmpValue right = EmitFunctionExpresion(BinOp->Right, Fn);
    TmpValue result = {};
    result.Type = left.Type;

    TypeError(left.Type, right.Type, BinOp->FileName, BinOp->Line);

    switch (BinOp->Op) {
    case AST::BinaryOperation::Add:
        MATH_NOT_EQUAL(result, left, right, Add);
        break;
    case AST::BinaryOperation::AddEqual:
        MATH_EQUAL(left, right, Add);
        break;
    case AST::BinaryOperation::Sub:
        MATH_NOT_EQUAL(result, left, right, Sub);
        break;
    case AST::BinaryOperation::SubEqual:
        MATH_EQUAL(left, right, Sub);
        break;
    case AST::BinaryOperation::Mul:
        MATH_NOT_EQUAL(result, left, right, Mul);
        break;
    case AST::BinaryOperation::MulEqual:
        MATH_EQUAL(left, right, Mul);
        break;
    case AST::BinaryOperation::Div:
        MATH_NOT_EQUAL(result, left, right, Div);
        break;
    case AST::BinaryOperation::DivEqual:
        MATH_EQUAL(left, right, Div);
        break;
    case AST::BinaryOperation::Comparision:
    case AST::BinaryOperation::NotEqual:
    case AST::BinaryOperation::Less:
    case AST::BinaryOperation::LessEqual:
    case AST::BinaryOperation::Greater:
    case AST::BinaryOperation::GreaterEqual:
    {
        Uint8 rightReg = Uint8(-1);
        if (!right.IsRegister()) {
            rightReg = AllocateRegister();
            MoveTmp(Fn, rightReg, right);
        }
        else {
            rightReg = (Uint8)right.Data;
        }

        if (left.IsRegister()) {
            if (left.Type.IsInt()) {
                Assembler.ICmp(Fn, left.Reg, rightReg);
            }
            else if (left.Type.IsUInt()) {
                Assembler.Cmp(Fn, left.Reg, rightReg);
            }
            else if (left.Type.IsFloat()) {
                Assembler.FCmp(Fn, left.Reg, rightReg);
            }
        }
        else if (left.IsLocal()) {
            Uint8 tmp = AllocateRegister();
            MoveTmp(Fn, tmp, left);
            DeallocateRegister(tmp);

            if (left.Type.IsInt()) {
                Assembler.ICmp(Fn, tmp, rightReg);
            }
            else if (left.Type.IsUInt()) {
                Assembler.Cmp(Fn, tmp, rightReg);
            }
            else if (left.Type.IsFloat()) {
                Assembler.FCmp(Fn, tmp, rightReg);
            }
        }

        if (rightReg != Uint8(-1)) {
            DeallocateRegister(rightReg);
        }
    }
        break;
    default:
        assert(0 && "Invalid binary operation");
        break;
    }

    result.LastOp = BinOp->Op;
    return result;
}

TmpValue Emitter::EmitFunctionUnary(AST::Unary* Unary, JKFunction& Fn) {
    return TmpValue();
}

TmpValue Emitter::EmitFunctionDot(AST::Dot* Dot, JKFunction& Fn) {
    return TmpValue();
}

TmpValue Emitter::EmitFunctionArrayList(AST::ArrayList* ArrayList, JKFunction& Fn) {
    return TmpValue();
}

TmpValue Emitter::EmitFunctionBlock(AST::Block* Block, JKFunction& Fn) {
    for (auto& stat : Block->Statements) {
        EmitFunctionStatement(stat, Fn);
    }

    return TmpValue();
}

void Emitter::PushTmp(JKFunction& Fn, const TmpValue& Tmp) {
    if (Tmp.IsRegister()) {
        Assembler.RPush(Fn, Tmp.Reg);
        DeallocateRegister(Tmp.Reg);
    }
    else if (Tmp.IsLocal()) {
        Assembler.LPush(Fn, Tmp.Local);
    }
    else if (Tmp.IsConstant()) {
        if (Tmp.Type.SizeInBits == 8)
            Assembler.Push8(Fn, (Uint8)Tmp.Data);
        else if (Tmp.Type.SizeInBits == 16)
            Assembler.Push16(Fn, (Uint16)Tmp.Data);
        else if (Tmp.Type.SizeInBits == 32)
            Assembler.Push32(Fn, (Uint32)Tmp.Data);
        else if (Tmp.Type.SizeInBits == 64)
            Assembler.Push64(Fn, Tmp.Data);
    }
}

void Emitter::MoveTmp(JKFunction& Fn, Uint8 Reg, const TmpValue& Tmp) {
    if (Tmp.IsRegister()) {
        Assembler.Mov(Fn, Reg, Tmp.Reg);
    }
    else if (Tmp.IsLocal()) {
        Assembler.LocalGet(Fn, Reg, Tmp.Local);
    }
    else if (Tmp.IsGlobal()) {
        Assembler.GlobalGet(Fn, Reg, (Uint32)Tmp.Data);
    }
    else if (Tmp.IsConstant()) {
        if (Tmp.Type.SizeInBits == 8)
            Assembler.Mov8(Fn, Reg, (Uint8)Tmp.Data);
        else if (Tmp.Type.SizeInBits == 16)
            Assembler.Mov16(Fn, Reg, (Uint16)Tmp.Data);
        else if (Tmp.Type.SizeInBits == 32)
            Assembler.Mov32(Fn, Reg, (Uint32)Tmp.Data);
        else if (Tmp.Type.SizeInBits == 64)
            Assembler.Mov64(Fn, Reg, Tmp.Data);
    }
}

}
