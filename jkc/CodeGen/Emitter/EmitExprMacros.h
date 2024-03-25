#pragma once

#define MAKE_OP(Type, Function, ...) \
    if(Type.IsInt()) {\
        State.CodeAssembler.I##Function(Fn, __VA_ARGS__);\
    }\
    else if(Type.IsUInt()) {\
        State.CodeAssembler.##Function(Fn, __VA_ARGS__);\
    }\
    else if (Type.IsFloat()) {\
        State.CodeAssembler.F##Function(Fn, __VA_ARGS__);\
    }

#define MAKE_CASE(Case) \
    if(Op == AST::BinaryOperation::Case || Op == AST::BinaryOperation::Case##Equal){\
        if(Right.IsRegister()) {\
            MAKE_OP(Right.Type, Case, result.Reg, leftReg, Right.Reg);\
            State.DeallocateRegister(Right.Reg);\
        }\
        else if (Right.IsConstant()) {\
            if(Right.Data == 1 && \
            (AST::BinaryOperation::Case == AST::BinaryOperation::Sub || \
             AST::BinaryOperation::Case == AST::BinaryOperation::SubEqual)) {\
                if(result.Reg != leftReg && (result.IsRegister() || result.IsLocalReg())) {\
                    State.CodeAssembler.Mov(Fn, result.Reg, leftReg);\
                }\
                MAKE_OP(Right.Type, Dec, result.Reg);\
            }\
            else if(Right.Data == 1 && \
            (AST::BinaryOperation::Case == AST::BinaryOperation::Add || \
             AST::BinaryOperation::Case == AST::BinaryOperation::AddEqual)) {\
                if(result.Reg != leftReg && (result.IsRegister() || result.IsLocalReg())) {\
                    State.CodeAssembler.Mov(Fn, result.Reg, leftReg);\
                }\
                MAKE_OP(Right.Type, Inc, result.Reg);\
            }\
            else if (Right.Data <= ByteMax) {\
                State.CodeAssembler.##Case##8(Fn, result.Reg, leftReg, Byte(Right.Data));\
            }\
            else if (Right.Data <= Const16Max) {\
                State.CodeAssembler.##Case##16(Fn, result.Reg, leftReg, UInt16(Right.Data));\
            }\
            else {\
                Byte tmp = State.AllocateRegister();\
                State.MoveTmp(Fn, tmp, Right);\
                MAKE_OP(Right.Type, Case, result.Reg, leftReg, tmp);\
                State.DeallocateRegister(tmp);\
            }\
        }\
        else if (Right.IsLocal()) {\
            Byte tmp = State.AllocateRegister();\
            State.MoveTmp(Fn, tmp, Right);\
            MAKE_OP(Right.Type, Case, result.Reg, leftReg, tmp);\
            State.DeallocateRegister(tmp);\
        }\
        else if (Right.IsLocalReg()) {\
            MAKE_OP(Right.Type, Case, result.Reg, leftReg, Right.Reg);\
        }\
    }

#define MAKE_BINARY_CASE(Case) \
    if(Op == AST::BinaryOperation::Binary##Case || Op == AST::BinaryOperation::Binary##Case##Equal){\
        if(Right.IsRegister()) {\
            State.CodeAssembler.##Case(Fn, result.Reg, leftReg, Right.Reg);\
            State.DeallocateRegister(Right.Reg);\
        }\
        else if (Right.IsConstant()) {\
            if (Right.Data <= ByteMax) {\
                State.CodeAssembler.##Case##8(Fn, result.Reg, leftReg, UInt8(Right.Data));\
            }\
            else if (Right.Data <= Const16Max) {\
                State.CodeAssembler.##Case##16(Fn, result.Reg, leftReg, UInt16(Right.Data));\
            }\
            else {\
                Byte tmp = State.AllocateRegister();\
                State.MoveTmp(Fn, tmp, Right);\
                State.CodeAssembler.##Case(Fn, result.Reg, leftReg, tmp);\
                State.DeallocateRegister(tmp);\
            }\
        }\
        else if (Right.IsLocal()) {\
            Byte tmp = State.AllocateRegister();\
            State.MoveTmp(Fn, tmp, Right);\
            State.CodeAssembler.##Case(Fn, result.Reg, leftReg, tmp);\
            State.DeallocateRegister(tmp);\
        }\
        else if (Right.IsLocalReg()) {\
            State.CodeAssembler.##Case(Fn, result.Reg, leftReg, Right.Reg);\
        }\
    }

#define MAKE_BINARY_CASE_NO_16(Case) \
    if(Op == AST::BinaryOperation::Binary##Case || Op == AST::BinaryOperation::Binary##Case##Equal){\
        if(Right.IsRegister()) {\
            State.CodeAssembler.##Case(Fn, result.Reg, leftReg, Right.Reg);\
            State.DeallocateRegister(Right.Reg);\
        }\
        else if (Right.IsConstant()) {\
            if (Right.Data <= ByteMax) {\
                State.CodeAssembler.##Case##8(Fn, result.Reg, leftReg, UInt8(Right.Data));\
            }\
            else {\
                Byte tmp = State.AllocateRegister();\
                State.MoveTmp(Fn, tmp, Right);\
                State.CodeAssembler.##Case(Fn, result.Reg, leftReg, tmp);\
                State.DeallocateRegister(tmp);\
            }\
        }\
        else if (Right.IsLocal()) {\
            Byte tmp = State.AllocateRegister();\
            State.MoveTmp(Fn, tmp, Right);\
            State.CodeAssembler.##Case(Fn, result.Reg, leftReg, tmp);\
            State.DeallocateRegister(tmp);\
        }\
        else if (Right.IsLocalReg()) {\
            State.CodeAssembler.##Case(Fn, result.Reg, leftReg, Right.Reg);\
        }\
    }

