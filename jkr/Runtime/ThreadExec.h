#pragma once
#include "jkr/Runtime/Stack.h"
#include "jkr/Runtime/Assembly.h"
#include "jkr/CodeFile/OpCodes.h"

namespace runtime {

struct [[nodiscard]] ThreadExecution {

    static ThreadExecution New(USize StackSize, Assembly& Asm);
    
    Value Exec(this ThreadExecution& Self, unsigned int Index, Value* SP);

    void ExecCommon(this ThreadExecution& Self, const  Function& Fn, codefile::OpCode C, 
                    StackFrame& Frame, UInt& IP);

    void Destroy(this ThreadExecution& Self);

    Stack ThreadStack;
    Assembly& Asm;
};


}
