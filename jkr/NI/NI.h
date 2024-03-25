#ifndef __JKR_NI_NI_H__
#define __JKR_NI_NI_H__

#if defined(_MSC_VER)
    #define JK_EXPORT __declspec(dllexport)
    #define JK_IMPORT __declspec(dllimport)
#endif // _MSC_VER

#if defined(JK_BUILD)
    #define JK_API JK_EXPORT
#else
    #define JK_API JK_IMPORT
#endif // JK_BUILD

#ifdef __cplusplus
extern "C" {
#endif //

typedef unsigned char JKByte;
typedef JKByte JKBool;
typedef long long JKInt;
typedef unsigned long long JKUInt;
typedef double JKFloat;
typedef JKUInt JKObject;

typedef void* JKOpaque;
typedef JKOpaque JKAssembly;
typedef JKOpaque JKVirtualMachine;
typedef JKByte* JKString;
typedef JKOpaque JKArray;

typedef enum {
    JK_OK,
    JK_CORRUPT_ASM,
    JK_ASM_NOT_FOUND,
    JK_ASM_BAD_FILE,
    
    JK_VM_LINKAGE_ERROR,
    JK_VM_STACK_OVERFLOW,

} JKResult;

// Assembly

JK_API JKResult jkrLoadAssembly(JKString Path, JKAssembly* pAsm);

JK_API void jkrUnloadAssembly(JKAssembly Asm);

// Virtual Machine

JK_API JKResult jkrCreateVM(JKVirtualMachine* pVM, JKUInt StackSize);

JK_API void jkrVMSetAssembly(JKVirtualMachine VM, JKAssembly Asm);

JK_API JKResult jkrVMLink(JKVirtualMachine VM);

JK_API JKResult jkrVMExecuteMain(JKVirtualMachine VM, JKInt* ExitValue);

JK_API void jkrDestroyVM(JKVirtualMachine VM);

// Runtime

JK_API JKResult jkrCreateObject(JKUInt ObjectType, JKObject* pObject);

JK_API JKResult jkrGetField(JKObject Object, JKByte Index, JKObject* pField);

JK_API JKOpaque jkrArrayBytes(JKArray ArrayRef);

JK_API JKByte jkrArrayGetByte(JKArray ArrayRef, JKUInt Index);

JK_API JKInt jkrArrayGetInt(JKArray ArrayRef, JKUInt Index);

JK_API JKUInt jkrArrayGetUInt(JKArray ArrayRef, JKUInt Index);

JK_API JKFloat jkrArrayGetFloat(JKArray ArrayRef, JKUInt Index);

#ifdef __cplusplus
}
#endif //

#endif // __JKR_NI_C_H__
