#pragma once

namespace AST {

enum class BinaryOperation {
    None,
    Assignment,

    Add,
    Inc,
    Sub,
    Dec,
    Mul,
    Div,
    AddEqual,
    SubEqual,
    MulEqual,
    DivEqual,
    
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
};

}
