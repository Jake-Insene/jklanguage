#include "jkr/Runtime/Assembly.h"
#include <jkr/CodeFile/Type.h>
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

    UInt64& sig = *(UInt64*)codefile::Signature;
    if (loadedAssembly->Signature != sig) {
        loadedAssembly->Err = AsmCorruptFile;
        file.Close();
        return loadedAssembly;
    }

    loadedAssembly->Sections.GrowCapacity(loadedAssembly->NumberOfSections);
    for (Byte i = 0; i < loadedAssembly->NumberOfSections; i++) {
        Section& sc = loadedAssembly->Sections.Push();
        file.Read(Cast<Byte*>(&sc), sizeof(codefile::SectionHeader));

        if (sc.Type == codefile::SectionCode) {
            loadedAssembly->CodeSection = &sc;
            if (loadedAssembly->EntryPoint >= sc.CountOfElements) {
                loadedAssembly->Err = AsmBadFile;
                file.Close();
                return loadedAssembly;
            }

            sc.Functions.GrowCapacity(sc.CountOfElements);
            for (UInt32 f = 0; f < sc.CountOfElements; f++) {
                auto& fn = sc.Functions.Push();
                file.Read(Cast<Byte*>(&fn), sizeof(codefile::FunctionHeader));
                if (fn.Flags == codefile::FunctionNative) {
                    continue;
                }

                fn.Code.GrowCapacity(fn.SizeOfCode);
                file.Read(Cast<Byte*>(fn.Code.Data), IntCast<USize>(fn.SizeOfCode));
                fn.Code.Size = fn.SizeOfCode;
                fn.Asm = loadedAssembly;
            }
        }
        else if (sc.Type == codefile::SectionData) {
            loadedAssembly->DataSection = &sc;
            sc.Data.GrowCapacity(sc.CountOfElements);
            for (UInt32 g = 0; g < sc.CountOfElements; g++) {
                auto& element = sc.Data.Push();
                file.Read(Cast<Byte*>(&element), sizeof(codefile::DataHeader));
                if (element.Primitive == codefile::PrimitiveByte) {
                    file.Read(Cast<Byte*>(&element.Contant), 1);
                }
                else if (element.Primitive == codefile::PrimitiveInt && element.Primitive == codefile::PrimitiveFloat) {
                    file.Read(Cast<Byte*>(&element.Contant), 8);
                }
            }
        }
        else if (sc.Type == codefile::SectionST) {
            loadedAssembly->STSection = &sc;
            sc.Strings.GrowCapacity(sc.CountOfElements);
            for (UInt32 s = 0; s < sc.CountOfElements; s++) {
                UInt16 size = 0;
                file.Read(Cast<Byte*>(&size), 2);

                auto& str = sc.Strings.Push(size, 1);
                str.ItemType = codefile::PrimitiveByte;

                file.Read(Cast<Byte*>(str.Items), size);
                str.Size = size;
            }
        }
    }

    file.Close();

    return loadedAssembly;
}

void Assembly::Destroy(this Assembly& Self) {
    Self.Sections.Destroy();
    mem::Deallocate(&Self);
}

}
