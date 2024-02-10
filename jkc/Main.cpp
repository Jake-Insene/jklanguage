#include <jkc/Compiler.h>
#include <jkr/Runtime/VirtualMachine.h>
#include <jkr/Lib/Error.h>

#include <iostream>
#include <chrono>

std::chrono::high_resolution_clock::time_point start;
std::chrono::high_resolution_clock::time_point end;

static inline void PrintDuration() {
    Str sufix = STR("mcs");
    Int duration = 0;
    auto dmcs = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    if (dmcs >= 1000) {
        sufix = STR("ms");
        auto dms = dmcs / 1'000;
        if (dms >= 1000) {
            duration = dmcs / 1'000'000;
            sufix = STR("s");
        }
        else
            duration = dms;
    }
    else
        duration = dmcs;

    io::Print(STR("{i}{s}"), duration, sufix);
}

static inline void BeginAction(Str FileName, ActionType Type, bool Error) {
    start = std::chrono::high_resolution_clock::now();
    if (Type == ActionType::Parsing && !Error) {
        io::Println(FileName);
    }
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
    {
        ProfileData pd = {
            .BeginAction = BeginAction,
            .EndAction = EndAction,
        };

        io::File err = io::GetStderr();
        Compiler compiler = Compiler::New(err, pd);

        CompileResult result = compiler.CompileFromSource(STR("Examples/main.jkl"));
        if (!result.Success) {
            io::Println(STR("Compilation fail"));
            error::Exit(UInt(-1));
        }
    }

    runtime::Assembly as = runtime::Assembly::FromFile(STR("Examples/main.jk"));
    runtime::VirtualMachine vm = runtime::VirtualMachine::New(1024 * 1024, as);
    
    start = std::chrono::high_resolution_clock::now();
    UInt exitCode = vm.ExecMain().Unsigned;
    end = std::chrono::high_resolution_clock::now();

    io::Print(STR("[Exectuion tooks "));
    PrintDuration();
    io::Println(STR("]"));

    io::Println(STR("Program exit with code {u}"), exitCode);
    
    vm.Destroy();
    as.Destroy();

    std::cin.get();
    return 0;
}
