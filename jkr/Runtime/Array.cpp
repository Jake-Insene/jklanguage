#include "jkr/Runtime/Array.h"

namespace runtime {

void Array::Destroy(this Array& Self) {
    mem::Deallocate(Self.Items);
}

}
