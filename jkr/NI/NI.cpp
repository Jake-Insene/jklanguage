#include "jkr/NI/NI.h"
#include "jkr/Runtime/Object.h"
#include "jkr/Runtime/Assembly.h"
#include "jkr/Runtime/VirtualMachine.h"

extern "C" JK_API JKResult jkrLoadAssembly(JKString Path, JKAssembly * pAsm) {
    runtime::Assembly* loadedAssembly = new runtime::Assembly(Str(Path));
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
    runtime::Assembly* as = reinterpret_cast<runtime::Assembly*>(Asm);
    delete as;
}

extern "C" JK_API JKResult jkrCreateVM(JKVirtualMachine* pVM, JKUInt StackSize) {
    runtime::VirtualMachine* vm = new runtime::VirtualMachine(StackSize, nullptr);
    
    *pVM = vm;
    return JK_OK;
}

extern "C" JK_API void jkrVMSetAssembly(JKVirtualMachine VM, JKAssembly Asm) {
    runtime::VirtualMachine* vm = reinterpret_cast<runtime::VirtualMachine*>(VM);
    vm->Asm = reinterpret_cast<runtime::Assembly*>(Asm);
    vm->LinkageResolved = false;
}

JK_API JKResult jkrVMLink(JKVirtualMachine VM) {
    runtime::VirtualMachine* vm = reinterpret_cast<runtime::VirtualMachine*>(VM);
    vm->ResolveExtern();
    if (vm->Err != runtime::VMSuccess) {
        if (vm->Err == runtime::VMLinkageError) {
            return JK_VM_LINKAGE_ERROR;
        }
    }

    return JK_OK;
}

extern "C" JK_API JKResult jkrVMExecuteMain(JKVirtualMachine VM, JKInt* ExitValue) {
    runtime::VirtualMachine* vm = reinterpret_cast<runtime::VirtualMachine*>(VM);
    *ExitValue = vm->ExecMain();
    if (vm->Err != runtime::VMSuccess) {
        if (vm->Err == runtime::VMLinkageError) {
            return JK_VM_LINKAGE_ERROR;
        }
    }

    return JK_OK;
}

extern "C" JK_API void jkrDestroyVM(JKVirtualMachine VM) {
    runtime::VirtualMachine* vm = reinterpret_cast<runtime::VirtualMachine*>(VM);
    delete vm;
}

extern "C" JK_API JKResult jkrCreateObject(JKUInt ObjectType, JKObject* pObject) {
    return JKResult();
}

extern "C" JK_API JKResult jkrGetField(JKObject Object, JKByte Index, JKObject* pField) {
    return JKResult();
}

extern "C" JK_API JKOpaque jkrArrayBytes(JKArray ArrayRef) {
    runtime::Array* array = reinterpret_cast<runtime::Array*>(ArrayRef);
    return array->Bytes;
}

extern "C" JK_API JKByte jkrArrayGetByte(JKArray ArrayRef, JKUInt Index) {
    runtime::Array* array = reinterpret_cast<runtime::Array*>(ArrayRef);
    return array->GetByte(Index);
}

extern "C" JK_API JKInt jkrArrayGetInt(JKArray ArrayRef, JKUInt Index) {
    runtime::Array* array = reinterpret_cast<runtime::Array*>(ArrayRef);
    return array->GetInt(Index);
}

extern "C" JK_API JKUInt jkrArrayGetUInt(JKArray ArrayRef, JKUInt Index) {
    runtime::Array* array = reinterpret_cast<runtime::Array*>(ArrayRef);
    return array->GetUInt(Index);
}

extern "C" JK_API JKFloat jkrArrayGetFloat(JKArray ArrayRef, JKUInt Index) {
    runtime::Array* array = reinterpret_cast<runtime::Array*>(ArrayRef);
    return array->GetFloat(Index);
}
