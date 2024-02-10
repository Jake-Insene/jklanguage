#pragma once
#include "jkr/Runtime/ThreadExec.h"
#include "jkr/Runtime/Assembly.h"

namespace runtime {

struct [[nodiscard]] VirtualMachine {
    
    static VirtualMachine JK_API New(USize StackSize, Assembly& Asm);

    Value JK_API ExecMain(this VirtualMachine& Self);

    void JK_API Destroy(this VirtualMachine& Self);

    ThreadExecution MainThread;
    Assembly& Asm;
};

}
