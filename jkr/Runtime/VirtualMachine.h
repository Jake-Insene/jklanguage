#pragma once
#include "jkr/Runtime/ThreadExec.h"
#include "jkr/Runtime/Assembly.h"
#include "jkr/Runtime/Library.h"

namespace runtime {

enum VMError {
    VMSucess = 0,
    VMLinkageError = 1,
};

struct [[nodiscard]] VirtualMachine {
    
    static VirtualMachine* New(USize StackSize, Assembly* Asm);

    Value ExecMain(this VirtualMachine& Self);
    void ResolveExtern(this VirtualMachine& Self);
    bool TryLoad(this VirtualMachine& Self, ByteList& LibName, Library& Lib);

    void Destroy(this VirtualMachine& Self);

    ThreadExecution MainThread;
    Assembly* Asm;
    List<Library> Libraries;
    Stack VMStack;
    VMError Err;
};

}
