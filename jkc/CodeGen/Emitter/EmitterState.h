#pragma once
#include "jkc/AST/Program.h"
#include "jkc/AST/Type.h"
#include "jkc/AST/Enums.h"
#include "jkc/CodeGen/Assembler.h"
#include <jkr/String.h>
#include <jkr/CodeFile/Array.h>
#include <jkr/CodeFile/Type.h>
#include <iostream>
#include <cassert>

#define CHECK_UNINITIALIZED_LOCAL(Local, Location) \
    if (!(Local).IsInitialized) {\
        State.Error(Location, u8"Trying to use a uninitialized var");\
    }

namespace CodeGen {

enum class FileType {
    Executable = 0,
    Library = 1,
};

enum DebugLevel {
    DBG_NONE,
    DBG_NORMAL,
};

enum Optimization {
    OPTIMIZATION_NONE,
    OPTIMIZATION_RELEASE_FAST,
};

struct EmitOptions {
    DebugLevel Debug;
    Optimization OptimizationLevel;
};

struct RegisterInfo {
    Byte Index = 0;
    bool IsAllocated = false;
};

struct [[nodiscard]] EmitterState {
    EmitterState(FILE* ErrorStream);
    ~EmitterState();

    void Emit(AST::Program& Program, FileType FileTy, EmitOptions Options, std::ostream& Output);

    // Error/Warning utility
    void Warn(const SourceLocation& Location, Str Format, ...);
    void Error(const SourceLocation& Location, Str Format, ...);
    void TypeError(const AST::TypeDecl& LHS, const AST::TypeDecl& RHS,
                   const SourceLocation& Location, Str Format, ...);
    void GlobalError(Str Format, ...);

    // Utility/Helper functions
    TmpValue GetID(const StringView& ID, Function& Fn, const SourceLocation& Location);
    Function* GetFn(AST::Expresion* Target);

    // Utility/Helper for tmp values
    void PushTmp(Function& Fn, const TmpValue& Tmp);
    void MoveTmp(Function& Fn, UInt8 Reg, const TmpValue& Tmp);
    codefile::PrimitiveType TypeToPrimitive(const AST::TypeDecl& Type);
    codefile::ArrayElement TypeToArrayElement(const AST::TypeDecl& Type);
    void MoveConst(Function& Fn, Byte Dest, UInt64 Const);

    // Register
    UInt8 AllocateRegister() {
        for (auto& r : Registers) {
            if (!r.IsAllocated) {
                r.IsAllocated = true;
                return r.Index;
            }
        }
        assert(0 && "Register allocation fail");
        return UInt8(-1);
    }

    void DeallocateRegister(UInt8 Index) {
        assert(Index <= 15 && "Invalid register index");
        Registers[Index].IsAllocated = false;
    }

    FILE* ErrorStream;
    bool Success = true;
    EmitOptions CurrentOptions;

    Assembler CodeAssembler;
    struct {
        bool IsInReturn;
        bool IsInCall;
        bool IsLast;
        bool IsInIf;
        bool IsInElse;
    } Context;

    RegisterInfo Registers[32] = {
        {0, false},
        {1, false},
        {2, false},
        {3, false},
        {4, false},
        {5, false},
        {6, false},
        {7, false},
        {8, false },
        {9, false },
        {10, false },
        {11, false },
        {12, false },
        {13, false },
        {14, false },
        {15, false },
        {16, false },
        {17, false },
        {18, false },
        {19, false },
        {20, false },
        {21, false },
        {22, false },
        {23, false },
        {24, false },
        {25, false },
        {26, false },
        {27, false },
        {28, false },
        {29, false },
        {30, false },
        {31, false },
    };

    RegisterInfo VectorRegisters[16] = {
        {0, false},
        {1, false},
        {2, false},
        {3, false},
        {4, false},
        {5, false},
        {6, false},
        {7, false},
        {8, false },
        {9, false },
        {10, false },
        {11, false },
        {12, false },
        {13, false },
        {14, false },
        {15, false },
    };

    SymbolTable<Function> Functions;
    SymbolTable<Global> Globals;
    std::unordered_map<StringView, UInt32> NativeLibraries;
    std::vector<StringTmp> Strings;
};

}

