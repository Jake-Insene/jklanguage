#pragma once
#include "stdjk/CoreHeader.h"
#include "jkr/Runtime/Value.h"
#include "jkr/Runtime/DataElement.h"

namespace runtime {

struct [[nodiscard]] Stack {

    static Stack New(USize StackSize);

    void Destroy(this Stack& Self);

    USize Size;
    Value* Start;
};

struct [[nodiscard]] StackFrame {
    Value* SP;
    Value* FP;
    DataElement* DataElements;
    // Store the result value of a call
    Value Result;
};

}
