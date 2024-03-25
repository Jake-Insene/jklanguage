#pragma once
#include "jkr/Runtime/Assembly.h"
#include "jkr/Runtime/Library.h"
#include "jkr/Runtime/Stack.h"

namespace runtime {

enum VMError {
    VMSuccess = 0,
    VMLinkageError = 1,
};

struct [[nodiscard]] VirtualMachine {
    VirtualMachine(USize StackSize, Assembly* Asm);
    ~VirtualMachine();

    Int ExecMain();
    
    void ResolveExtern();

    bool TryLoad(Array& LibName, USize& Lib);

    UInt ProcessCall(StackFrame& Frame, Function& Fn);

    UInt MainLoop(Function& Fn, StackFrame& Frame);

    Stack VMStack;
    Assembly* Asm;
    std::vector<Library> Libraries;
    VMError Err;
    bool LinkageResolved;
};

}
