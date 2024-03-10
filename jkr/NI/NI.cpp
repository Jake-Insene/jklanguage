#include "jkr/Runtime/Object.h"
#include "jkr/NI/ThreadState.h"
#include "jkr/NI/NI.h"
#include "jkr/Runtime/Assembly.h"
#include "jkr/Runtime/VirtualMachine.h"

extern "C" JK_API JKResult jkrLoadAssembly(JKString Path, JKAssembly * pAsm) {
    runtime::Assembly* loadedAssembly = runtime::Assembly::FromFile(Cast<Str>(Path));
    if (loadedAssembly->Err != runtime::AsmOk) {
        if (loadedAssembly->Err == runtime::AsmCorruptFile) {
            return JK_CORRUPT_ASM;
        }
        else if (loadedAssembly->Err == runtime::AsmBadFile) {
            return JK_ASM_BAD_FILE;
        }
        else if (loadedAssembly->Err == runtime::AsmNotExists) {
            return JK_ASM_NOT_FOUND;
        }
    }

    *pAsm = loadedAssembly;
    return JK_OK;
}

extern "C" JK_API void jkrUnloadAssembly(JKAssembly Asm) {
    runtime::Assembly* as = Cast<runtime::Assembly*>(Asm);
    as->Destroy();
}

extern "C" JK_API JKResult jkrCreateVM(JKVirtualMachine* pVM, JKUInt StackSize, JKUInt LocalSize) {
    runtime::VirtualMachine* vm = runtime::VirtualMachine::New(StackSize, LocalSize, nullptr);
    
    *pVM = vm;
    return JK_OK;
}

extern "C" JK_API void jkrVMSetAssembly(JKVirtualMachine VM, JKAssembly Asm) {
    runtime::VirtualMachine* vm = Cast<runtime::VirtualMachine*>(VM);
    vm->Asm = Cast<runtime::Assembly*>(Asm);
}

JK_API JKResult jkrVMLink(JKVirtualMachine VM) {
    runtime::VirtualMachine* vm = Cast<runtime::VirtualMachine*>(VM);
    vm->ResolveExtern();
    if (vm->Err != runtime::VMSucess) {
        if (vm->Err == runtime::VMLinkageError) {
            return JK_VM_LINKAGE_ERROR;
        }
    }

    return JK_OK;
}

extern "C" JK_API JKResult jkrVMExecuteMain(JKVirtualMachine VM, JKValue* ExitValue) {
    runtime::VirtualMachine* vm = Cast<runtime::VirtualMachine*>(VM);
    ExitValue->U = vm->ExecMain().Unsigned;
    if (vm->Err != runtime::VMSucess) {
        if (vm->Err == runtime::VMLinkageError) {
            return JK_VM_LINKAGE_ERROR;
        }
    }

    return JK_OK;
}

extern "C" JK_API void jkrDestroyVM(JKVirtualMachine VM) {
    runtime::VirtualMachine* vm = Cast<runtime::VirtualMachine*>(VM);
    vm->Destroy();
}

extern "C" JK_API JKResult jkrCreateObject(JKThreadState State, JKUInt ObjectType, JKObject* pObject) {
    return JKResult();
}

extern "C" JK_API JKResult jkrGetField(JKThreadState State, JKObject Object, JKByte Index, JKObject* pField) {
    return JKResult();
}

extern "C" JK_API JKOpaque jkrArrayBytes(JKThreadState State, JKArray ArrayRef) {
    runtime::Array* array = Cast<runtime::Array*>(ArrayRef);
    return array->Items;
}

extern "C" JK_API JKValue jkrArrayGet(JKThreadState State, JKArray ArrayRef, JKUInt Index) {
    return JKValue();
}
