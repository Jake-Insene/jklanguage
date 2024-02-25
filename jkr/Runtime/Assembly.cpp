#include "jkr/Runtime/Assembly.h"
#include <stdjk/IO/File.h>
#include <stdjk/Mem/Allocator.h>
#include <malloc.h>

namespace runtime {

Assembly* Assembly::FromFile(Str FilePath) {
    Assembly* loadedAssembly = Cast<Assembly*>(
        mem::Allocate(sizeof(Assembly))
    );
    loadedAssembly->FilePath = FilePath;

    io::File file = io::File::Open(FilePath);
    if (file.Err == io::NotExists) {
        loadedAssembly->Err = AsmNotExists;
        return loadedAssembly;
    }

    if (file.Size() < sizeof(codefile::FileHeader)) {
        loadedAssembly->Err = AsmBadFile;
        file.Close();
        return loadedAssembly;
    }

    file.Read(Cast<Byte*>(loadedAssembly), sizeof(codefile::FileHeader));
    if (file.Size() != loadedAssembly->CheckSize) {
        loadedAssembly->Err = AsmCorruptFile;
        file.Close();
        return loadedAssembly;
    }

    UInt32& sig = *(UInt32*)codefile::Signature;
    if (loadedAssembly->Signature != sig) {
        loadedAssembly->Err = AsmCorruptFile;
        file.Close();
        return loadedAssembly;
    }

    if (loadedAssembly->EntryPoint >= loadedAssembly->NumberOfFunctions) {
        loadedAssembly->Err = AsmBadFile;
        file.Close();
        return loadedAssembly;
    }

    loadedAssembly->Functions.GrowCapacity(loadedAssembly->NumberOfFunctions);
    for (codefile::FunctionType i = 0; i < loadedAssembly->NumberOfFunctions; i++) {
        auto& fn = loadedAssembly->Functions.Push();
        file.Read(Cast<Byte*>(&fn), sizeof(codefile::FunctionHeader));
        if (fn.Attributes & codefile::FunctionNative) {
            continue;
        }

        fn.Code.GrowCapacity(fn.SizeOfCode);
        file.Read(Cast<Byte*>(fn.Code.Data), IntCast<USize>(fn.SizeOfCode));
        fn.Code.Size = fn.SizeOfCode;
        fn.Asm = loadedAssembly;
    }

    loadedAssembly->Globals.GrowCapacity(loadedAssembly->NumberOfGlobals);
    for (codefile::GlobalType i = 0; i < loadedAssembly->NumberOfGlobals; i++) {
        auto& global = loadedAssembly->Globals.Push();
        file.Read(Cast<Byte*>(&global), sizeof(codefile::GlobalHeader));

        if (global.Primitive == codefile::PrimitiveByte) {
            file.Read(Cast<Byte*>(&global.Contant), 1);
        }
        else if (global.Primitive >= codefile::PrimitiveInt && global.Primitive <= codefile::PrimitiveFloat) {
            file.Read(Cast<Byte*>(&global.Contant), 8);
        }
    }

    loadedAssembly->Strings.GrowCapacity(loadedAssembly->NumberOfStrings);
    for (codefile::StringType i = 0; i < loadedAssembly->NumberOfStrings;i++) {
        UInt16 size = 0;
        file.Read(Cast<Byte*>(&size), 2);

        auto& s = loadedAssembly->Strings.Push(size);
        s.ListType = ListOfBytes;

        Char* asmStr = Cast<Char*>(alloca(size));
        asmStr[size-1] = 0;
        file.Read((Byte*)asmStr, size);

        s = ByteList::New(size);
        mem::Copy(s.Elements.Data, Cast<Byte*>(asmStr), size);
    }

    file.Close();

    return loadedAssembly;
}

void Assembly::Destroy(this Assembly& Self) {
    Self.Functions.Destroy();
    Self.Globals.Destroy();
    Self.Strings.Destroy();

    mem::Deallocate(&Self);
}

}
