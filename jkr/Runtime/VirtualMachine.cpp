#include "jkr/Runtime/VirtualMachine.h"
#include <stdjk/Debug.h>
#include <stdjk/IO/Output.h>
#include <stdjk/Mem/Allocator.h>

namespace runtime {

VirtualMachine* VirtualMachine::New(USize StackSize, Assembly* Asm) {
    VirtualMachine* vm = Cast<VirtualMachine*>(
        mem::Allocate(sizeof(VirtualMachine))
    );

    vm->MainThread = ThreadExecution::New(vm);
    vm->Asm = Asm;
    vm->Libraries = List<Library>::New(0);
    vm->VMStack = Stack::New(StackSize);

    return vm;
}

Value VirtualMachine::ExecMain(this VirtualMachine& Self) {
    if (Self.Asm->Err != AsmOk) return {};
    Self.ResolveExtern();
    if (Self.Err != VMSucess) {
        return {};
    }
    Function* fn = &Self.Asm->Functions[Self.Asm->EntryPoint];
    return Self.MainThread.Exec(fn, Self.VMStack.Start);
}

void VirtualMachine::ResolveExtern(this VirtualMachine& Self) {
    for (auto& fn : Self.Asm->Functions) {
        if (fn.Attributes & codefile::FunctionNative) {
            Library lib = {};
            if (!Self.TryLoad(Self.Asm->Strings[fn.LIndex], lib)) {
                Self.Err = VMLinkageError;
                return;
            }

            Str entry = Cast<Str>(Self.Asm->Strings[fn.EIndex].Elements.Data);
            fn.Native = (Value(*)(ThreadState*, ...))lib.Get(entry);
            if (!fn.Native) {
                Self.Err = VMLinkageError;
                return;
            }
        }
    }
}

bool VirtualMachine::TryLoad(this VirtualMachine& Self, ByteList& LibName, Library& Lib) {
    for (auto& lib : Self.Libraries) {
        USize len = Strlen(lib.FilePath);
        if (len != LibName.Elements.Size) {
            continue;
        }

        if (memcmp(lib.FilePath, LibName.Elements.Data, len) == 0) {
            Lib = lib;
            return true;
        }
    }
    
    Char buff[1024] = {};
    if constexpr (Platform == Windows) {
        sprintf_s((char*)buff, 1024, "%s.dll", (char*)LibName.Elements.Data);
    }
    else {
        sprintf((char*)buff, "lib%s.so", (char*)LibName.Elements.Data);
    }

    Lib = Self.Libraries.Push((Char*)buff);
    if (!Lib.Handle)
        return false;

    return true;
}

void VirtualMachine::Destroy(this VirtualMachine& Self) {
    Self.MainThread.Destroy();
    Self.Libraries.Destroy();

    mem::Deallocate(&Self);
}

}
