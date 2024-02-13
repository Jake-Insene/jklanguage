#include "jkr/Runtime/VirtualMachine.h"
#include "jkr/Lib/Debug.h"
#include "jkr/IO/Output.h"

namespace runtime {

VirtualMachine JK_API VirtualMachine::New(USize StackSize, Assembly& Asm) {
    return VirtualMachine{
        .MainThread = ThreadExecution::New(StackSize, Asm),
        .Asm = Asm,
    };
}

Value JK_API VirtualMachine::ExecMain(this VirtualMachine& Self) {
    if (Self.Asm.Err != AsmOk) return {};
    return Self.MainThread.Exec(Self.Asm.Header.EntryPoint, Self.MainThread.ThreadStack.Start);
}

void JK_API VirtualMachine::Destroy(this VirtualMachine& Self) {
    Self.MainThread.Destroy();
}

}
