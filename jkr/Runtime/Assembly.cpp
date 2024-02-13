#include "jkr/Runtime/Assembly.h"
#include "jkr/IO/File.h"

namespace runtime {

Assembly JK_API Assembly::FromFile(Str FilePath) {
    Assembly loadedAssembly = {
        .FilePath = FilePath,
    };

    io::File file = io::File::Open(FilePath, io::FileRead | io::FileBinary);
    if (file.Size() < sizeof(codefile::FileHeader)) {
        loadedAssembly.Err = AsmCorruptFile;
        return loadedAssembly;
    }

    file.Read(Cast<Byte*>(&loadedAssembly.Header), sizeof(loadedAssembly.Header));
    if (file.Size() != loadedAssembly.Header.CheckSize) {
        loadedAssembly.Err = AsmCorruptFile;
        return loadedAssembly;
    }

    if (loadedAssembly.Header.EntryPoint >= loadedAssembly.Header.CountOfFunctions) {
        loadedAssembly.Err = AsmCorruptFile;
        return loadedAssembly;
    }

    for (UInt i = 0; i < loadedAssembly.Header.CountOfFunctions; i++) {
        auto& fn = loadedAssembly.Functions.Push();
        file.Read(Cast<Byte*>(&fn.Header), sizeof(codefile::FunctionHeader));
        fn.Code.GrowCapacity(fn.Header.SizeOfCode);
        file.Read(Cast<Byte*>(fn.Code.Data), IntCast<USize>(fn.Header.SizeOfCode));
        fn.Code.Size = fn.Header.SizeOfCode;
    }

    file.Close();

    return loadedAssembly;
}

}
