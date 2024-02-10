#pragma once
#include "jkr/CoreHeader.h"
#include "jkr/Runtime/Value.h"

namespace runtime {

struct [[nodiscard]] Stack {

    static Stack New(USize StackSize);

    void Destroy(this Stack& Self);

    USize Size;
    Value* Start;
};

struct [[nodiscard]] StackFrame {
    Value* SP;
    Value* Globals;
    Value* FP;
    // Store the result value of a call
    Value Result;
};

}
