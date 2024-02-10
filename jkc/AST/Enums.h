#pragma once

namespace AST {

enum class BinaryOperation {
    None,
    Assignment,

    Add,
    AddEqual,
    Inc,
    Sub,
    SubEqual,
    Dec,
    Mul,
    MulEqual,
    Div,
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
