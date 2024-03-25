#pragma once

namespace AST {

enum class BinaryOperation {
    None,

    Add,
    Sub,
    Mul,
    Div,
    BinaryAnd,
    BinaryOr,
    BinaryXOr,
    BinaryShl,
    BinaryShr,
    AddEqual,
    SubEqual,
    MulEqual,
    DivEqual,
    BinaryAndEqual,
    BinaryOrEqual,
    BinaryXOrEqual,
    BinaryShlEqual,
    BinaryShrEqual,

    Comparision,
    NotEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
};

enum class UnaryOperation {
    None,
    Negate,
    LogicalNegate,
    BinaryNAND,
};

}
