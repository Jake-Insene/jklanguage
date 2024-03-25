#include "jkr/Runtime/Array.h"
#include "jkr/Definitions.h"
#include "jkr/Align.h"

namespace runtime {

constexpr UInt16 ElementToSize(codefile::ArrayElement Type) {
    switch (Type) {
    case codefile::AE_1B:
        return 1;
    case codefile::AE_8B:
        return 8;
    case codefile::AT_32B:
        return 32;
    default:
        break;
    }

    return 0;
}

Array::Array(USize Size, codefile::ArrayElement ElementType) :
    Size(Size), ElementSize(ElementSize), ElementType(ElementType) {
    ElementSize = ElementToSize(ElementType);
    Bytes = new Byte[Size * ElementSize]{};
}

Array::~Array() {
    delete[] Bytes;
}

}
