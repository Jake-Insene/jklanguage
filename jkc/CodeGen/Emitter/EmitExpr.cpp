#include "jkc/CodeGen/Emitter/EmitExpr.h"
#include "jkc/CodeGen/Emitter/EmitStat.h"
#include "jkc/CodeGen/Emitter/EmitterState.h"
#include "jkc/CodeGen/Emitter/EmitExprMacros.h"
#include "jkc/AST/Expresions.h"
#include <jkr/Utility.h>

namespace CodeGen {

TmpValue EmitExpresion(EmitterState& /*State*/, AST::Expresion* Expr) {
    if (Expr->Type == AST::ExpresionType::Constant) {
        auto constant = (AST::Constant*)Expr;
        return TmpValue{
            .Ty = TmpType::Constant,
            .Data = constant->Unsigned,
            .Type = constant->ValueType,
        };
    }

    return TmpValue(TmpType::Err);
}

TmpValue EmitFunctionExpresion(EmitterState& State, AST::Expresion* Expr, Function& Fn) {
    if (Expr->Type == AST::ExpresionType::Constant) {
        return EmitFunctionConstant(State, (AST::Constant*)Expr, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::Identifier) {
        return EmitFunctionIdentifier(State, (AST::Identifier*)Expr, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::Group) {
        return EmitFunctionExpresion(State, ((AST::Group*)Expr)->Value.get(), Fn);
    }
    else if (Expr->Type == AST::ExpresionType::Call) {
        return EmitFunctionCall(State, (AST::Call*)Expr, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::BinaryOp) {
        return EmitFunctionBinaryOp(State, (AST::BinaryOp*)Expr, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::Unary) {
        return EmitFunctionUnary(State, (AST::Unary*)Expr, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::Dot) {
        return EmitFunctionDot(State, (AST::Dot*)Expr, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::ArrayList) {
        return EmitFunctionArrayList(State, (AST::ArrayList*)Expr, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::Block) {
        return EmitFunctionBlock(State, (AST::Block*)Expr, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::ArrayAccess) {
        return EmitFunctionArrayAccess(State, (AST::ArrayAccess*)Expr, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::IncDec) {
        return EmitFunctionIncDec(State, (AST::IncDec*)Expr, Fn);
    }
    else if (Expr->Type == AST::ExpresionType::Assignment) {
        return EmitFunctionAssignment(State, (AST::Assignment*)Expr, Fn);
    }

    assert(0 && "Invalid expresion");
    return TmpValue(TmpType::Err);
}

TmpValue EmitFunctionConstant(EmitterState& State, AST::Constant* Constant, Function& /*Fn*/) {
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
    else if (Constant->ValueType.IsConstString()) {
        auto& str = State.Strings.emplace_back(Constant->String.data(), Constant->String.size());
        tmp.Data = State.Strings.size() - 1;
        str.Location = Constant->Location;
    }

    return tmp;
}

TmpValue EmitFunctionIdentifier(EmitterState& State, AST::Identifier* ID, Function& Fn) {
    return State.GetID(ID->ID, Fn, ID->Location);
}

TmpValue EmitFunctionCall(EmitterState& State, AST::Call* Call, Function& Fn) {
    TmpValue result = {};
    Function* target = State.GetFn(Call->Target.get());
    if (target == nullptr) {
        return TmpValue{ TmpType::Err };
    }

    std::vector<Byte> usedRegisters{};
    for (auto& reg : State.Registers) {
        if (reg.IsAllocated) {
            usedRegisters.emplace_back(reg.Index);
            State.CodeAssembler.Push(Fn, reg.Index);
            State.DeallocateRegister(reg.Index);
        }
    }

    if (target->CountOfArguments != Call->Arguments.size()) {
        State.Error(Call->Location,
              u8"%llu arguments was expected but %llu was founded",
              (UInt)target->CountOfArguments,
              Call->Arguments.size()
        );
        return TmpValue(TmpType::Err);
    }

    State.Context.IsInCall = true;
    for (Int8 i = target->CountOfArguments; i > 0; i--) {
        TmpValue arg = EmitFunctionExpresion(
            State, Call->Arguments[static_cast<USize>(i) - 1].get(), Fn
        );

        // Checking uninitialized variables
        if (arg.IsFunctionLocal()) {
            CHECK_UNINITIALIZED_LOCAL(Fn.Locals.Get(arg.Index), Call->Location);
        }

        if (target->IsRegisterBased()) {
            if (i <= target->RegisterArguments) {
                if (arg.IsLocalReg() || arg.IsRegister()) {
                    if (arg.Reg != i) {
                        State.CodeAssembler.Mov(Fn, Byte(i), arg.Reg);
                    }
                }
                else {
                    State.MoveTmp(Fn, Byte(i), arg);
                }
            }
        }
        else {
            State.PushTmp(Fn, arg);
        }

        auto& local = target->Locals.Get(
            static_cast<USize>(i) - 1
        );
        State.TypeError(
            local.Type, arg.Type, Call->Location,
            u8"Invalid argument at %d: using a value of type '%s' with a argument of type '%s'",
            i, arg.Type.ToString().c_str(), local.Type.ToString().c_str()
        );

    }
    State.Context.IsInCall = false;

    if (target->Address <= Const32Max) {
        State.CodeAssembler.Call(Fn, target->Address);
    }
    else {
        State.Error(Call->Location, u8"Address too long");
        return TmpValue(TmpType::Err);
    }

    if (usedRegisters.size()) {
        Int i = Int(usedRegisters.size());
        while (i > 0) {
            State.CodeAssembler.Pop(Fn, usedRegisters[i-1]);
            State.Registers[usedRegisters[i - 1]].IsAllocated = true;
            i--;
        }
    }

    result.Ty = TmpType::Register;
    result.Type = target->Type;
    if (!target->Type.IsVoid()) {
        State.Registers[0].IsAllocated = true;
        result.Reg = 0;
    }

    return result;
}

TmpValue EmitFunctionBinaryOp(EmitterState& State, AST::BinaryOp* BinOp, Function& Fn) {
    TmpValue left = EmitFunctionExpresion(State, BinOp->Left.get(), Fn);
    TmpValue right = EmitFunctionExpresion(State, BinOp->Right.get(), Fn);

    // Checking uninitialized variables
    if (left.IsFunctionLocal()) {
        CHECK_UNINITIALIZED_LOCAL(Fn.Locals.Get(left.Index), BinOp->Location);
    }
    if (right.IsFunctionLocal()) {
        CHECK_UNINITIALIZED_LOCAL(Fn.Locals.Get(right.Index), BinOp->Location);
    }

    if (left.IsErr() || right.IsErr()) {
        return TmpValue{ TmpType::Err };
    }

    TmpValue result = left;
    State.TypeError(
        left.Type, right.Type, BinOp->Location,
        u8"Invalid operands type '%s' and '%s'",
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
    case AST::BinaryOperation::BinaryAnd:
    case AST::BinaryOperation::BinaryAndEqual:
    case AST::BinaryOperation::BinaryOr:
    case AST::BinaryOperation::BinaryOrEqual:
    case AST::BinaryOperation::BinaryXOr:
    case AST::BinaryOperation::BinaryXOrEqual:
    case AST::BinaryOperation::BinaryShl:
    case AST::BinaryOperation::BinaryShlEqual:
    case AST::BinaryOperation::BinaryShr:
    case AST::BinaryOperation::BinaryShrEqual:
        result = EmitBinaryOp(State, left, right, BinOp->Op, Fn);
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
                State.CodeAssembler.TestZ(Fn, left.Reg);
            }
            else if (left.IsLocal()) {
                UInt8 tmp = State.AllocateRegister();
                State.MoveTmp(Fn, tmp, left);
                State.CodeAssembler.TestZ(Fn, tmp);
                State.DeallocateRegister(tmp);
            }
            else if (left.IsLocalReg()) {
                State.CodeAssembler.TestZ(Fn, left.Reg);
            }
            break;
        }

        if (!right.IsLocalReg() && !right.IsRegister()) {
            rightReg = State.AllocateRegister();
            State.MoveTmp(Fn, rightReg, right);
        }
        else {
            rightReg = (UInt8)right.Data;
        }

        UInt8 leftReg = UInt8(-1);
        if (left.IsRegister() || left.IsLocalReg()) {
            leftReg = left.Reg;
        }
        else if (left.IsLocal() || left.IsGlobal()) {
            leftReg = State.AllocateRegister();
            State.MoveTmp(Fn, leftReg, left);
            State.DeallocateRegister(leftReg);
        }

        if (left.Type.IsUInt() || left.Type.IsInt()) {
            State.CodeAssembler.Cmp(Fn, leftReg, rightReg);
        }
        else if (left.Type.IsFloat()) {
            State.CodeAssembler.FCmp(Fn, leftReg, rightReg);
        }

        if (rightReg != UInt8(-1)) {
            State.DeallocateRegister(rightReg);
        }
    }
    break;
    default:
        assert(0 && "Invalid binary operation");
        return TmpValue(TmpType::Err);
    }

    result.Type = left.Type;
    result.LastOp = BinOp->Op;
    return result;
}

TmpValue EmitBinaryOp(EmitterState& State, TmpValue& Left, TmpValue& Right, AST::BinaryOperation Op, Function& Fn) {
    TmpValue result = {};
    result.Reg = Byte(-1);
    UInt8 leftReg = Byte(-1);
    UInt asInt = UInt(Op);

    if (asInt >= 10 && asInt <= 18) {
        result = Left;
        if (!Left.IsLocalReg() && !Left.IsRegister()) {
            leftReg = State.AllocateRegister();
            State.MoveTmp(Fn, leftReg, Left);
            result.Reg = leftReg;
        }
        else {
            result.Ty = Left.Ty;
            result.Reg = leftReg = Left.Reg;
        }
    }
    else {
        result.Ty = TmpType::Register;
        if (State.Context.IsInReturn && !State.Context.IsInCall) {
            if (!State.Registers[0].IsAllocated) {
                result.Reg = 0;
                State.Registers[0].IsAllocated = true;
            }
            else if ((Left.Reg == 0 && Left.IsRegister()) || (Right.Reg == 0 && Right.IsRegister())) {
                result.Reg = 0;
            }
        }

        if (!Left.IsLocalReg() && !Left.IsRegister()) {
            leftReg = State.AllocateRegister();
            State.MoveTmp(Fn, leftReg, Left);
            if (result.Reg == Byte(-1)) {
                result.Reg = leftReg;
            }
        }
        else {
            leftReg = Left.Reg;
        }

        if (result.Reg == Byte(-1)) {
            if (Left.IsRegister()) {
                result.Reg = Left.Reg;
            }
            else {
                result.Reg = State.AllocateRegister();
            }
        }
    }

    MAKE_CASE(Add);
    MAKE_CASE(Sub);
    MAKE_CASE(Mul);
    MAKE_CASE(Div);
    MAKE_BINARY_CASE(And);
    MAKE_BINARY_CASE(Or);
    MAKE_BINARY_CASE(XOr);
    MAKE_BINARY_CASE_NO_16(Shl);
    MAKE_BINARY_CASE_NO_16(Shr);

    if (asInt >= 10 && asInt <= 18) {
        if (!Left.IsLocalReg() && !Left.IsRegister()) {
            if (Left.IsLocal()) {
                State.CodeAssembler.LocalSet(Fn, leftReg, Left.Local);
            }
            else if (Left.IsGlobal()) {
                State.CodeAssembler.GlobalSet(Fn, leftReg, Left.Global);
            }
            State.DeallocateRegister(leftReg);
        }
    }
    else {
        if (Left.IsRegister() && result.Reg != Left.Reg) {
            State.DeallocateRegister(Left.Reg);
        }
        else if (!Left.IsLocalReg() && !Left.IsRegister()) {
            State.DeallocateRegister(leftReg);
        }
    }

    return result;
}

TmpValue EmitFunctionUnary(EmitterState& State, AST::Unary* Unary, Function& Fn) {
    TmpValue tmp = EmitFunctionExpresion(State, Unary->Value.get(), Fn);
    if (tmp.IsErr()) {
        return TmpValue{ TmpType::Err };
    }
    TmpValue result = {};
    result.Type = tmp.Type;
    result.Ty = TmpType::Register;

    // Checking uninitialized variables
    if (tmp.IsFunctionLocal()) {
        CHECK_UNINITIALIZED_LOCAL(Fn.Locals.Get(tmp.Index), Unary->Location);
    }

    if (tmp.IsLocalReg() || tmp.IsRegister()) {
        if (tmp.IsRegister()) {
            result.Reg = tmp.Reg;
        }
        else {
            result.Reg = State.AllocateRegister();
            State.CodeAssembler.Mov(Fn, result.Reg, tmp.Reg);
        }
    }
    else if(tmp.IsGlobal()){
        result.Reg = State.AllocateRegister();
        State.CodeAssembler.GlobalGet(Fn, result.Reg, tmp.Global);
    }
    else if (tmp.IsLocal()) {
        result.Reg = State.AllocateRegister();
        State.CodeAssembler.LocalGet(Fn, result.Reg, tmp.Local);
    }

    switch (Unary->Op) {
    case AST::UnaryOperation::Negate:
        State.CodeAssembler.Neg(Fn, result.Reg);
        break;
    case AST::UnaryOperation::LogicalNegate:
        State.CodeAssembler.Not(Fn, result.Reg);
        break;
    case AST::UnaryOperation::BinaryNAND:
        State.CodeAssembler.Not(Fn, result.Reg);
        break;
    default:
        assert(0 && "Invalid binary operation");
        return TmpValue(TmpType::Err);
    }

    return tmp;
}

TmpValue EmitFunctionDot(EmitterState& /*State*/, AST::Dot* /*Dot*/, Function& /*Fn*/) {
    return TmpValue();
}

TmpValue EmitFunctionArrayList(EmitterState& /*State*/, AST::ArrayList* ArrayList, Function& /*Fn*/) {
    return TmpValue{
        .Ty = TmpType::ArrayExpr,
        .Data = UInt(ArrayList),
    };
}

TmpValue EmitFunctionBlock(EmitterState& State, AST::Block* Block, Function& Fn) {
    for (auto& stat : Block->Statements) {
        EmitFunctionStatement(State, stat.get(), Fn);
    }

    return TmpValue();
}

TmpValue EmitFunctionArrayAccess(EmitterState& State, AST::ArrayAccess* ArrayAccess, Function& Fn) {
    TmpValue toIndex = EmitFunctionExpresion(State, ArrayAccess->Expr.get(), Fn);
    TmpValue index = EmitFunctionExpresion(State, ArrayAccess->IndexExpr.get(), Fn);
    if (toIndex.IsErr() || index.IsErr()) {
        return TmpValue(TmpType::Err);
    }

    // Checking uninitialized variables
    // A array is always initialized with its elements in 0 by default
    if (index.IsFunctionLocal()) {
        CHECK_UNINITIALIZED_LOCAL(Fn.Locals.Get(index.Index), ArrayAccess->Location);
    }

    if (toIndex.IsConstant()) {
        State.Error(ArrayAccess->Location, u8"Invalid indexing of a non constant array");
        return TmpValue(TmpType::Err);
    }

    Byte dest = Byte(-1);
    Byte arrayReg = Byte(-1);
    Byte indexReg = Byte(-1);

    if (toIndex.IsRegister() || toIndex.IsLocalReg()) {
        arrayReg = toIndex.Reg;
    }
    else {
        arrayReg = State.AllocateRegister();
        State.MoveTmp(Fn, arrayReg, toIndex);
    }


    if (index.IsRegister() || index.IsLocalReg()) {
        indexReg = index.Reg;
    }
    else {
        indexReg = State.AllocateRegister();
        State.MoveTmp(Fn, indexReg, index);
    }

    if (State.Context.IsInReturn && !State.Context.IsInCall) {
        if (arrayReg == 0) {
            dest = arrayReg;
        }
        else if (indexReg == 0) {
            dest = indexReg;
        }
        else {
            dest = State.AllocateRegister();
        }
    }
    else if (toIndex.IsRegister()) {
        dest = arrayReg;
    }
    else {
        dest = indexReg;
    }

    State.CodeAssembler.ArrayLoad(Fn, arrayReg, indexReg, dest);

    if (toIndex.IsRegister()) {
        State.DeallocateRegister(arrayReg);
    }
    if (index.IsRegister()) {
        State.DeallocateRegister(indexReg);
    }

    AST::TypeDecl elementType = toIndex.Type;
    elementType.Flags ^= AST::TypeDecl::Array;
    elementType.ArrayLen = 0;
    return TmpValue{
        .Ty = TmpType::Register,
        .Reg = dest,
        .Type = elementType,
    };
}

TmpValue EmitFunctionIncDec(EmitterState& State, AST::IncDec* IncDec, Function& Fn) {
    TmpValue tmp = EmitFunctionExpresion(State, IncDec->Expr.get(), Fn);
    if (tmp.IsErr()) {
        return TmpValue(TmpType::Err);
    }

    // A array is always initialized with its elements in 0 by default
    if (tmp.IsFunctionLocal()) {
        CHECK_UNINITIALIZED_LOCAL(Fn.Locals.Get(tmp.Index), IncDec->Location);
    }

    TmpValue result = {};
    result.Type = tmp.Type;

    Byte reg = 0;
    if (tmp.IsRegister() || tmp.IsLocalReg()) {
        if (IncDec->After && tmp.IsLocalReg()) {
            result.Ty = TmpType::Register;
            result.Reg = State.AllocateRegister();
            State.CodeAssembler.Mov(Fn, result.Reg, tmp.Reg);
            if (tmp.IsRegister()) {
                State.DeallocateRegister(tmp.Reg);
            }
            reg = tmp.Reg;
        }
        else {
            result = tmp;
            reg = result.Reg;
        }

    }
    else {
        reg = State.AllocateRegister();
        if (IncDec->After) {
            result.Ty = TmpType::Register;
            result.Reg = State.AllocateRegister();
            if (State.Context.IsInReturn) {
                if (reg == 0) {
                    Byte c = reg;
                    reg = result.Reg;
                    result.Reg = c;
                }
            }

            State.MoveTmp(Fn, result.Reg, tmp);
            State.CodeAssembler.Mov(Fn, reg, result.Reg);
        }
        else {
            result = tmp;
            State.MoveTmp(Fn, reg, tmp);
        }
    }

    if (IncDec->Increment) {
        if (tmp.Type.IsInt()) {
            State.CodeAssembler.IInc(Fn, reg);
        }
        else if (tmp.Type.IsUInt() || tmp.Type.IsByte()) {
            State.CodeAssembler.Inc(Fn, reg);
        }
        else if (tmp.Type.IsFloat()) {
            State.CodeAssembler.FInc(Fn, reg);
        }
    }
    else {
        if (tmp.Type.IsInt()) {
            State.CodeAssembler.IDec(Fn, reg);
        }
        else if (tmp.Type.IsUInt() || tmp.Type.IsByte()) {
            State.CodeAssembler.Dec(Fn, reg);
        }
        else if (tmp.Type.IsFloat()) {
            State.CodeAssembler.FDec(Fn, reg);
        }
    }

    if (!tmp.IsRegister() && !tmp.IsLocalReg()) {
        if (tmp.IsLocal()) {
            State.CodeAssembler.LocalSet(Fn, reg, tmp.Local);
        }
        else if (tmp.IsGlobal()) {
            State.CodeAssembler.GlobalSet(Fn, reg, tmp.Global);
        }
        else {
            State.Error(IncDec->Location, u8"Invalid operation");
            return TmpValue(TmpType::Err);
        }
    }

    State.DeallocateRegister(reg);

    return result;
}

TmpValue EmitFunctionAssignment(EmitterState& State, AST::Assignment* Assignment, Function& Fn) {
    TmpValue target = EmitFunctionExpresion(State, Assignment->Target.get(), Fn);
    TmpValue source = EmitFunctionExpresion(State, Assignment->Source.get(), Fn);
    if (target.IsErr() || source.IsErr()) {
        return TmpValue(TmpType::Err);
    }

    if (source.IsFunctionLocal()) {
        CHECK_UNINITIALIZED_LOCAL(Fn.Locals.Get(source.Index), Assignment->Location);
    }

    if (target.IsFunctionLocal()) {
        Fn.Locals.Get(target.Index).IsInitialized = true;
    }

    Byte sourceReg = Byte(-1);
    if (source.IsLocalReg() || source.IsRegister()) {
        sourceReg = source.Reg;
    }
    else if (source.IsArrayExpr()) {
        State.Error(Assignment->Location, u8"Invalid assignment");
        return TmpValue(TmpType::Err);
    }
    else if (source.IsConstant() && target.IsLocalReg()) {
        State.MoveTmp(Fn, target.Reg, source);
    }
    else {
        sourceReg = State.AllocateRegister();
        State.MoveTmp(Fn, sourceReg, source);

        if (target.IsLocalReg()) {
            State.CodeAssembler.Mov(Fn, target.Reg, sourceReg);
        }
        else if (target.IsLocal()) {
            State.CodeAssembler.LocalSet(Fn, sourceReg, target.Local);
        }
        else if (target.IsGlobal()) {
            State.CodeAssembler.GlobalSet(Fn, sourceReg, target.Global);
        }

        if (sourceReg != Byte(-1)) {
            State.DeallocateRegister(sourceReg);
        }
    }

    return target;
}

}
