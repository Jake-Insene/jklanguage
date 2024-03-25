#pragma once
#include "jkr/Runtime/Value.h"

namespace runtime {

struct [[nodiscard]] Stack {
    Stack(USize StackSize);
    ~Stack();

    Stack(Stack&&) = default;
    Stack& operator=(Stack&&) = default;

    USize Size;
    Value* Start;
};

struct [[nodiscard]] StackFrame {
    Value* SP;
    Value* FP;
    // Maybe use
    Value Result;
};

}
