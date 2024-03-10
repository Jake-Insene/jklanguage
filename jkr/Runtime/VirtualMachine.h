#pragma once
#include "jkr/Runtime/Assembly.h"
#include "jkr/Runtime/Library.h"
#include "jkr/Runtime/Stack.h"

namespace runtime {

enum VMError {
    VMSucess = 0,
    VMLinkageError = 1,
};

struct [[nodiscard]] VirtualMachine {
    using LocalStack = List<Value, false>;

    static VirtualMachine* New(USize StackSize, UInt LocalSize, Assembly* Asm);

    Value ExecMain(this VirtualMachine& Self);
    
    void ResolveExtern(this VirtualMachine& Self);

    bool TryLoad(this VirtualMachine& Self, Array& LibName, Library& Lib);

    Value ProcessCall(this VirtualMachine& Self, StackFrame& Frame, Function* Fn);

    Value MainLoop(this VirtualMachine& Self, Function* Fn, StackFrame& Frame);

    void Destroy(this VirtualMachine& Self);

    Stack VMStack;
    LocalStack Locals;
    Assembly* Asm;
    List<Library> Libraries;
    VMError Err;
    bool LinkageResolved;
};

}
