#include "jkr/Mem/Allocator.h"

extern Address __mem__allocator_Allocate(USize Size);
extern void __mem__allocator_Deallocate(Address Mem, USize Size);

namespace mem {

Address JK_API Allocate(USize Size) {
    return __mem__allocator_Allocate(Size);
}

void JK_API Deallocate(Address Mem, USize Size) {
    __mem__allocator_Deallocate(Mem, Size);
}

}
