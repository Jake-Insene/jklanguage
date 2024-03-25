#include "jkr/Runtime/Assembly.h"
#include <jkr/CodeFile/Type.h>
#include <fstream>

namespace runtime {

Assembly::Assembly(Str FilePath) {
    this->FilePath = FilePath;

    std::ifstream file{ (const char*)FilePath, std::ios::ate | std::ios::binary };
    if (!file.is_open()) {
        Err = AsmNotExists;
        return;
    }

    USize size = file.tellg();
    file.seekg(0);
    if (size < sizeof(codefile::FileHeader)) {
        Err = AsmBadFile;
        file.close();
        return;
    }

    file.read(reinterpret_cast<char*>(this), sizeof(codefile::FileHeader));
    if (size != this->CheckSize) {
        Err = AsmCorruptFile;
        file.close();
        return;
    }

    UInt32& sig = *(UInt32*)codefile::Signature;
    if (this->Signature != sig) {
        Err = AsmCorruptFile;
        file.close();
        return;
    }

    if (this->DataSize) {
        CodeSection.reserve(this->DataSize);
        for (UInt32 i = 0; i < this->DataSize; i++) {
            auto& element = DataSection.emplace_back();
            file.read((char*)&element, sizeof(codefile::DataHeader));
            if (element.Primitive == codefile::PrimitiveByte) {
                file.read((char*)&element.Value.Unsigned, 1);
            }
            else if (element.Primitive >= codefile::PrimitiveInt && element.Primitive <= codefile::PrimitiveFloat) {
                file.read((char*)&element.Value.Unsigned, 8);
            }
        }
    }

    if (this->FunctionSize) {
        CodeSection.reserve(this->FunctionSize);
        if (EntryPoint >= this->FunctionSize) {
            Err = AsmBadFile;
            file.close();
            return;
        }

        for (UInt32 i = 0; i < this->FunctionSize; i++) {
            auto& fn = CodeSection.emplace_back();
            file.read((char*)&fn, sizeof(codefile::FunctionHeader));
            fn.Asm = this;
            if (fn.Flags & codefile::FunctionNative) {
                continue;
            }

            fn.Code.resize(fn.SizeOfCode);
            file.read((char*)fn.Code.data(), fn.SizeOfCode);
        }
    }

    if (this->StringsSize) {
        STSection.reserve(this->StringsSize);
        for (UInt32 i = 0; i < this->StringsSize; i++) {
            UInt16 sizeStr = 0;
            file.read((char*)&sizeStr, UInt16(2));

            auto& str = STSection.emplace_back(sizeStr, codefile::AE_1B);

            file.read((char*)str.Bytes, sizeStr);
            str.Size = sizeStr;
        }
    }

    Err = AsmOk;
    file.close();
}

Assembly::~Assembly() {}

}
