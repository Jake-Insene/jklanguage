#include <jkc/Compiler.h>
#include <jkr/NI/NI.h>
#include <jkr/Error.h>
#include <jkr/Runtime/Value.h>
#include <chrono>

std::chrono::high_resolution_clock::time_point start;
std::chrono::high_resolution_clock::time_point  end;

// 1 mb for the stack
constexpr JKUInt StackSize = (1024 * 1024) / sizeof(runtime::Value);

static inline void PrintDuration() {
    const char* sufix = "ns";
    Int duration = (end - start).count();

    if (duration >= 1000) {
        sufix = "mcs";
        duration = duration / 1'000;
        if (duration >= 1'000) {
            sufix = "ms";
            duration = duration / 1'000;
            if (duration >= 1'000) {
                sufix = "s";
                duration = duration / 1'000;
            }
        }
    }

    printf("%lli%s", duration, sufix);
}

static inline void BeginAction(const char* /*FileName*/, ActionType /*Type*/, bool /*Error*/) {
    start = std::chrono::high_resolution_clock::now();
}

static inline void EndAction(const char* /*FileName*/, ActionType Type, bool Error) {
    end = std::chrono::high_resolution_clock::now();

    if (!Error) {
        if (Type == ActionType::Parsing) {
            printf("[Parsing tooks ");
            PrintDuration();
            printf("]\n");
        }
        else if (Type == ActionType::CodeGen) {
            printf("[CodeGen tooks ");
            PrintDuration();
            printf("]\n");
        }
        else if (Type == ActionType::Disassembly) {
            printf("[Disassembly tooks ");
            PrintDuration();
            printf("]\n");
        }
    }
}

int main() {
    {
        ProfileData pd = {
            .BeginAction = BeginAction,
            .EndAction = EndAction,
        };

        Compiler compiler = Compiler(stderr, pd);

        CodeGen::EmitOptions options = {
            .Debug = CodeGen::DBG_NORMAL,
            .OptimizationLevel = CodeGen::OPTIMIZATION_RELEASE_FAST,
        };

        if (!compiler.CompileFromSource("Examples/main.jkl", options).Success) {
            puts("Compilation fail");
            error::Exit(UInt(-1));
        }

        FILE* output = nullptr;
        (void)fopen_s(&output, "Examples/main.jks", "wb");
        if (output == nullptr) {
            puts("Disassembly fail");
            error::Exit(UInt(-1));
        }

        if (!compiler.Disassembly("Examples/main.jk", output).Success) {
            puts("Disassembly fail");
            error::Exit(UInt(-1));
        }

        fclose(output);
    }

    JKResult result = JK_OK;
    JKAssembly as;
    result = jkrLoadAssembly(
        JKString(u8"Examples/main.jk"),
        &as
    );

    if (result != JK_OK) {
        puts("Error loading the file");
        error::Exit(UInt(-1));
    }

    JKVirtualMachine vm = {};
    result = jkrCreateVM(
        &vm,
        StackSize
    );
    jkrVMSetAssembly(vm, as);

    result = jkrVMLink(vm);
    if (result == JK_VM_LINKAGE_ERROR) {
        puts("VM: Linkage error");
        error::Exit(UInt(-1));
    }

    start = std::chrono::high_resolution_clock::now();
    JKInt exitValue;
    result = jkrVMExecuteMain(vm, &exitValue);
    end = std::chrono::high_resolution_clock::now();
    if (result != JK_OK) {
        if (result == JK_VM_STACK_OVERFLOW) {
            puts("VM: Stack overflow");
        }
        else {
            puts("VM Error");
        }
        error::Exit(UInt(-1));
    }

    printf("[Exectuion tooks ");
    PrintDuration();
    puts("]");

    printf("Program exit with code %llu\n", exitValue);

    jkrDestroyVM(vm);
    jkrUnloadAssembly(as);

    return 0;
}
