#pragma once
#include "jkr/Runtime/Stack.h"
#include "jkr/Runtime/Function.h"
#include "jkr/CodeFile/OpCodes.h"

namespace runtime {

struct VirtualMachine;

struct [[nodiscard]] ThreadExecution {

    static ThreadExecution New(VirtualMachine* VM);
    
    Value Exec(this ThreadExecution& Self, Function* Fn, Value* SP);

    Value ProcessCall(this ThreadExecution& Self, StackFrame& Frame, Function* Fn);

    void Destroy(this ThreadExecution& Self);

    VirtualMachine* VM;
};


}
