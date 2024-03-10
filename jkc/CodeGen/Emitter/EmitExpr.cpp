#include "jkc/CodeGen/Emitter/Emitter.h"
#include "jkc/AST/Expresions.h"

namespace CodeGen {

TmpValue Emitter::EmitExpresion(AST::Expresion* Expr) {
    if (Expr->Type == AST::ExpresionType::Constant) {
        auto constant = (AST::Constant*)Expr;
        return TmpValue{
            .Ty = TmpType::Constant,
            .Data = constant->Unsigned,
            .Type = constant->ValueType,
        };
    }

    return TmpValue();
}

TmpValue Emitter::EmitFunctionExpresion(AST::Expresion* Expr, Function& Fn) {
    if (Expr->Type == AST::ExpresionType::Constant) {
        return EmitFunctionConstant((AST::Constant*)Expr, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::Identifier) {
        return EmitFunctionIdentifier((AST::Identifier*)Expr, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::Group) {
        return EmitFunctionExpresion(((AST::Group*)Expr)->Value, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::Call) {
        return EmitFunctionCall((AST::Call*)Expr, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::BinaryOp) {
        return EmitFunctionBinaryOp((AST::BinaryOp*)Expr, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::Unary) {
        return EmitFunctionUnary((AST::Unary*)Expr, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::Dot) {
        return EmitFunctionDot((AST::Dot*)Expr, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::ArrayList) {
        return EmitFunctionArrayList((AST::ArrayList*)Expr, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::Block) {
        return EmitFunctionBlock((AST::Block*)Expr, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::ArrayAccess) {
        return EmitFunctionArrayAccess((AST::ArrayAccess*)Expr, Fn);
    }

    assert(0 && "Invalid expresion");
    return TmpValue();
}

TmpValue Emitter::EmitFunctionConstant(AST::Constant* Constant, Function& /*Fn*/) {
    TmpValue tmp = {};
    tmp.Ty = TmpType::Constant;
    tmp.Type = Constant->ValueType;
    tmp.Data = Constant->Unsigned;

    if (Constant->ValueType.IsUInt() || Constant->ValueType.IsInt()) {
        if (Constant->Unsigned <= Const4Max) {
            tmp.Type.SizeInBits = 4;
        }
        else if (Constant->Unsigned <= UINT8_MAX) {
            tmp.Type.SizeInBits = 8;
        }
        else if (Constant->Unsigned <= UINT16_MAX) {
            tmp.Type.SizeInBits = 16;
        }
        else if (Constant->Unsigned <= UINT32_MAX) {
            tmp.Type.SizeInBits = 32;
        }
    }
    else if (Constant->ValueType.IsConstString()) {
        Strings.Push(Constant->String.c_str(), Constant->String.size() + 1);
        tmp.Data = Strings.Size - 1;
    }

    return tmp;
}

TmpValue Emitter::EmitFunctionIdentifier(AST::Identifier* ID, Function& Fn) {
    return GetID(ID->ID, Fn, ID->Location);
}

TmpValue Emitter::EmitFunctionCall(AST::Call* Call, Function& Fn) {
    TmpValue result = {};
    Function* target = GetFn(Call->Target);
    if (target == nullptr) return result;

    List<Byte> usedRegisters;
    for (auto& reg : Registers) {
        if (reg.IsAllocated) {
            usedRegisters.Push(reg.Index);
            CodeAssembler.Push(Fn, reg.Index);
            DeallocateRegister(reg.Index);
        }
    }

    if (target->CountOfArguments != Call->Arguments.Size) {
        Error(Call->Location,
              STR("{u} arguments was expected but {u} was founded"),
              (UInt)target->CountOfArguments,
              Call->Arguments.Size
        );
    }

    Context.IsInCall = true;
    for (Int8 i = target->CountOfArguments; i > 0; i--) {
        TmpValue arg = EmitFunctionExpresion(
            Call->Arguments[static_cast<USize>(i) - 1], Fn
        );

        if (target->IsRegisterBased()) {
            if (i <= target->RegisterArguments) {
                if (arg.Ty == TmpType::Register) {
                    if (arg.Reg != i) {
                        CodeAssembler.Mov(Fn, Byte(i), arg.Reg);
                    }
                }
                else {
                    MoveTmp(Fn, Byte(i), arg);
                }
            }
        }
        else {
            PushTmp(Fn, arg);
        }

        auto& local = target->Locals.Get(
            static_cast<USize>(i) - 1
        );
        TypeError(
            local.Type, arg.Type, Call->Location,
            STR("Invalid argument at {d}: using a value of type '{s}' with a argument of type '{s}'"),
            i, arg.Type.ToString().c_str(), local.Type.ToString().c_str()
        );

    }
    Context.IsInCall = false;

    if (target->Address <= ByteMax) {
        CodeAssembler.Call8(Fn, (Byte)target->Address);
    }
    else {
        CodeAssembler.Call(Fn, target->Address);
    }

    if (usedRegisters.Size) {
        auto end = usedRegisters.end();
        while (--end != usedRegisters.begin() - 1) {
            CodeAssembler.Pop(Fn, *end);
            Registers[*end].IsAllocated = true;
        }
    }

    result.Ty = TmpType::Register;
    result.Type = target->Type;
    if (!target->Type.IsVoid()) {
        Registers[0].IsAllocated = true;
        result.Reg = 0;
    }

    usedRegisters.Destroy();
    return result;
}

TmpValue Emitter::EmitFunctionBinaryOp(AST::BinaryOp* BinOp, Function& Fn) {
    TmpValue left = EmitFunctionExpresion(BinOp->Left, Fn);
    TmpValue right = EmitFunctionExpresion(BinOp->Right, Fn);
    TmpValue result = {};
    TypeError(
        left.Type, right.Type, BinOp->Location,
        STR("Invalid operands type '{s}' and '{s}'"),
        left.Type.ToString().c_str(), right.Type.ToString().c_str()
    );

    switch (BinOp->Op) {
    case AST::BinaryOperation::Add:
    case AST::BinaryOperation::AddEqual:
    case AST::BinaryOperation::Sub:
    case AST::BinaryOperation::SubEqual:
    case AST::BinaryOperation::Mul:
    case AST::BinaryOperation::MulEqual:
    case AST::BinaryOperation::Div:
    case AST::BinaryOperation::DivEqual:
        result = EmitBinaryOp(left, right, BinOp->Op, Fn);
        break;
    case AST::BinaryOperation::Comparision:
    case AST::BinaryOperation::NotEqual:
    case AST::BinaryOperation::Less:
    case AST::BinaryOperation::LessEqual:
    case AST::BinaryOperation::Greater:
    case AST::BinaryOperation::GreaterEqual:
    {
        UInt8 rightReg = UInt8(-1);
        if (right.IsConstant() && right.Data == 0) {
            if (left.IsRegister()) {
                CodeAssembler.TestZ(Fn, left.Reg);
            }
            else if (left.IsLocal()) {
                UInt8 tmp = AllocateRegister();
                MoveTmp(Fn, tmp, left);
                CodeAssembler.TestZ(Fn, tmp);
                DeallocateRegister(tmp);
            }
            else if (left.IsLocalReg()) {
                CodeAssembler.TestZ(Fn, left.Reg);
            }
            break;
        }

        if (!right.IsLocalReg() && !right.IsRegister()) {
            rightReg = AllocateRegister();
            MoveTmp(Fn, rightReg, right);
        }
        else {
            rightReg = (UInt8)right.Data;
        }

        if (left.IsRegister()) {
            if (left.Type.IsInt()) {
                CodeAssembler.ICmp(Fn, left.Reg, rightReg);
            }
            else if (left.Type.IsUInt()) {
                CodeAssembler.Cmp(Fn, left.Reg, rightReg);
            }
            else if (left.Type.IsFloat()) {
                CodeAssembler.FCmp(Fn, left.Reg, rightReg);
            }
        }
        else if (left.IsLocal()) {
            UInt8 tmp = AllocateRegister();
            MoveTmp(Fn, tmp, left);
            DeallocateRegister(tmp);

            if (left.Type.IsInt()) {
                CodeAssembler.ICmp(Fn, tmp, rightReg);
            }
            else if (left.Type.IsUInt()) {
                CodeAssembler.Cmp(Fn, tmp, rightReg);
            }
            else if (left.Type.IsFloat()) {
                CodeAssembler.FCmp(Fn, tmp, rightReg);
            }
        }
        else if (left.IsLocalReg()) {
            if (left.Type.IsInt()) {
                CodeAssembler.ICmp(Fn, left.Reg, rightReg);
            }
            else if (left.Type.IsUInt()) {
                CodeAssembler.Cmp(Fn, left.Reg, rightReg);
            }
            else if (left.Type.IsFloat()) {
                CodeAssembler.FCmp(Fn, left.Reg, rightReg);
            }
        }

        if (rightReg != UInt8(-1)) {
            DeallocateRegister(rightReg);
        }
    }
    break;
    default:
        assert(0 && "Invalid binary operation");
        break;
    }

    result.Type = left.Type;
    result.LastOp = BinOp->Op;
    return result;
}

#define MAKE_OP_CONST(Type, Function, ...) \
    if(Type.IsInt()) {\
        CodeAssembler.I##Function(Fn, __VA_ARGS__);\
    }\
    else if(Type.IsUInt()) {\
        CodeAssembler.##Function(Fn, __VA_ARGS__);\
    }\

#define MAKE_OP(Type, Function, ...) \
    if(Type.IsInt()) {\
        CodeAssembler.I##Function(Fn, __VA_ARGS__);\
    }\
    else if(Type.IsUInt()) {\
        CodeAssembler.##Function(Fn, __VA_ARGS__);\
    }\
    else if (Type.IsFloat()) {\
        CodeAssembler.F##Function(Fn, __VA_ARGS__);\
    }

#define MAKE_CASE(Case) \
    if(Op == AST::BinaryOperation::Case){\
        if(Right.IsRegister()) {\
            MAKE_OP(Right.Type, Case, result.Reg, leftReg, Right.Reg);\
            DeallocateRegister(Right.Reg);\
        }\
        else if (Right.IsConstant()) {\
            if(Right.Data == 1 && \
            (AST::BinaryOperation::Case == AST::BinaryOperation::Sub || \
             AST::BinaryOperation::Case == AST::BinaryOperation::SubEqual)) {\
                MAKE_OP(Right.Type, Dec, leftReg);\
            }\
            else if(Right.Data == 1 && \
            (AST::BinaryOperation::Case == AST::BinaryOperation::Add || \
             AST::BinaryOperation::Case == AST::BinaryOperation::AddEqual)) {\
                MAKE_OP(Right.Type, Inc, leftReg);\
            }\
            else if (Right.Type.SizeInBits <= 8) {\
                MAKE_OP_CONST(Right.Type, Case##8, result.Reg, leftReg, Right.Reg);\
            }\
            else if (Right.Type.SizeInBits <= 16) {\
                MAKE_OP_CONST(Right.Type, Case##16, result.Reg, leftReg, (UInt16)Right.Data);\
            }\
            else {\
            Byte tmp = AllocateRegister();\
            MoveTmp(Fn, tmp, Right);\
            MAKE_OP(Right.Type, Case, result.Reg, leftReg, tmp);\
            DeallocateRegister(tmp);\
            }\
        }\
        else if (Right.IsLocal()) {\
            Byte tmp = AllocateRegister();\
            MoveTmp(Fn, tmp, Right);\
            MAKE_OP(Right.Type, Case, result.Reg, leftReg, tmp);\
            DeallocateRegister(tmp);\
        }\
        else if (Right.IsLocalReg()) {\
            MAKE_OP(Right.Type, Case, result.Reg, leftReg, Right.Reg);\
        }\
    }

TmpValue Emitter::EmitBinaryOp(TmpValue& Left, TmpValue& Right, AST::BinaryOperation Op, Function& Fn) {
    TmpValue result = {};
    result.Reg = Byte(-1);
    UInt asInt = UInt(Op);

    if (asInt >= 8 && asInt <= 11) {
    }
    else {
        UInt8 leftReg = Byte(-1);
        result.Ty = TmpType::Register;
        if (Context.IsInReturn && !Context.IsInCall) {
            if (!Registers[0].IsAllocated) {
                result.Reg = 0;
                Registers[0].IsAllocated = true;
            }
            else {
                if (Left.Reg == 0 || Right.Reg == 0)
                    result.Reg = 0;
            }
        }

        if (!Left.IsLocalReg() && !Left.IsRegister()) {
            leftReg = AllocateRegister();
            MoveTmp(Fn, leftReg, Left);
            if (result.Reg == Byte(-1))
                result.Reg = leftReg;
        }
        else {
            leftReg = Left.Reg;
            if (result.Reg == Byte(-1))
                result.Reg = Left.Reg;
        }

        MAKE_CASE(Add);
        MAKE_CASE(Sub);
        MAKE_CASE(Mul);
        MAKE_CASE(Div);

        if (!Left.IsLocalReg() && !Left.IsRegister()) {
            DeallocateRegister(leftReg);
        }
    }

    return result;
}

TmpValue Emitter::EmitFunctionUnary(AST::Unary* /*Unary*/, Function& /*Fn*/) {
    return TmpValue();
}

TmpValue Emitter::EmitFunctionDot(AST::Dot* /*Dot*/, Function& /*Fn*/) {
    return TmpValue();
}

TmpValue Emitter::EmitFunctionArrayList(AST::ArrayList* ArrayList, Function& /*Fn*/) {
    return TmpValue{
        .Ty = TmpType::ArrayExpr,
        .Data = Cast<UInt>(ArrayList),
    };
}

TmpValue Emitter::EmitFunctionBlock(AST::Block* Block, Function& Fn) {
    for (auto& stat : Block->Statements) {
        EmitFunctionStatement(stat, Fn);
    }

    return TmpValue();
}

TmpValue Emitter::EmitFunctionArrayAccess(AST::ArrayAccess* ArrayAccess, Function& Fn) {
    TmpValue toIndex = EmitFunctionExpresion(ArrayAccess->Expr, Fn);
    TmpValue index = EmitFunctionExpresion(ArrayAccess->IndexExpr, Fn);

    if (toIndex.IsConstant()) {
        Error(ArrayAccess->Location, STR("Invalid indexing of a non constant array"));
    }
    else {
        Byte dest = Byte(-1);
        Byte arrayReg = Byte(-1);
        Byte indexReg = Byte(-1);

        if (toIndex.IsRegister() || toIndex.IsLocalReg()) {
            arrayReg = toIndex.Reg;
        }
        else {
            arrayReg = AllocateRegister();
            MoveTmp(Fn, arrayReg, toIndex);
        }


        if (index.IsRegister() || index.IsLocalReg()) {
            indexReg = index.Reg;
        }
        else {
            indexReg = AllocateRegister();
            MoveTmp(Fn, indexReg, index);
        }

        if (toIndex.IsRegister()) {
            dest = arrayReg;
        }
        else {
            dest = indexReg;
        }

        CodeAssembler.ArrayGet(Fn, arrayReg, indexReg, dest);

        if (toIndex.IsRegister()) {
            DeallocateRegister(arrayReg);
        }
        if (index.IsRegister()) {
            DeallocateRegister(indexReg);
        }

        return TmpValue{
            .Ty = TmpType::Register,
            .Reg = dest,
            .Type = toIndex.Type,
        };
    }

    return TmpValue();
}

}
