#include <jkc/Compiler.h>
#include <jkr/NI/NI.h>
#include <stdjk/Error.h>
#include <chrono>

std::chrono::high_resolution_clock::time_point start;
std::chrono::high_resolution_clock::time_point  end;

constexpr JKUInt StackSize = (1028 * 64) / sizeof(JKValue);
constexpr JKUInt LocalSize = (1028 * 1024) / sizeof(JKValue);

static inline void PrintDuration() {
    Str sufix = STR("ns");
    Int duration = (end - start).count();

    if (duration >= 1000) {
        sufix = STR("mcs");
        duration = duration / 1'000;
        if (duration >= 1000) {
            sufix = STR("ms");
            duration = duration / 1'000;
            if (duration >= 1000) {
                sufix = STR("s");
                duration = duration / 100;
            }
        }
    }

    io::Print(STR("{i}{s}"), duration, sufix);
}

static inline void BeginAction(Str /*FileName*/, ActionType /*Type*/, bool /*Error*/) {
    start = std::chrono::high_resolution_clock::now();
}

static inline void EndAction(Str /*FileName*/, ActionType Type, bool Error) {
    end = std::chrono::high_resolution_clock::now();

    if (!Error) {
        if (Type == ActionType::Parsing) {
            io::Print(STR("[Parsing tooks "));
            PrintDuration();
            io::Println(STR("]"));
        }
        else if (Type == ActionType::CodeGen) {
            io::Print(STR("[CodeGen tooks "));
            PrintDuration();
            io::Println(STR("]"));
        }
        else if (Type == ActionType::Disassembly) {
            io::Print(STR("[Disassembly tooks "));
            PrintDuration();
            io::Println(STR("]"));
        }
    }
}

int main() {
    ProfileData pd = {
        .BeginAction = BeginAction,
        .EndAction = EndAction,
    };

    io::File err = io::GetStdout();
    Compiler compiler = Compiler::New(err, pd);

    CodeGen::EmitOptions options = {
        .Debug = CodeGen::DBG_NORMAL,
        .OptimizationLevel = CodeGen::OPTIMIZATION_RELEASE_FAST,
    };

    if (!compiler.CompileFromSource(STR("Examples/main.jkl"), options).Success) {
        io::Println(STR("Compilation fail"));
        error::Exit(UInt(-1));
    }

    io::File output = io::File::Create(STR("Examples/main.jks"));
    if (output.Err != io::Success) {
        io::Println(STR("Disassembly fail"));
        error::Exit(UInt(-1));
    }

    if (!compiler.Disassembly(STR("Examples/main.jk"), output).Success) {
        io::Println(STR("Disassembly fail"));
        error::Exit(UInt(-1));
    }

    compiler.Destroy();
    output.Close();

    JKResult result = JK_OK;
    JKAssembly as;
    result = jkrLoadAssembly(
        JKString(STR("Examples/main.jk")),
        &as
    );

    if (result != JK_OK) {
        io::Println(STR("Error loading the file"));
        error::Exit(UInt(-1));
    }

    JKVirtualMachine vm = {};
    result = jkrCreateVM(
        &vm,
        StackSize,
        LocalSize
    );
    jkrVMSetAssembly(vm, as);

    result = jkrVMLink(vm);
    if (result == JK_VM_LINKAGE_ERROR) {
        io::Println(STR("VM: Linkage error"));
        error::Exit(UInt(-1));
    }

    start = std::chrono::high_resolution_clock::now();
    JKValue exitValue;
    result = jkrVMExecuteMain(vm, &exitValue);
    end = std::chrono::high_resolution_clock::now();
    if (result != JK_OK) {
        if (result == JK_VM_STACK_OVERFLOW) {
            io::Println(STR("VM: Stack overflow"));
        }
        else {
            io::Println(STR("VM Error"));
        }
        error::Exit(UInt(-1));
    }

    io::Print(STR("[Exectuion tooks "));
    PrintDuration();
    io::Println(STR("]"));

    io::Println(STR("Program exit with code {u}"), exitValue.U);

    jkrDestroyVM(vm);
    jkrUnloadAssembly(as);

    return 0;
}
