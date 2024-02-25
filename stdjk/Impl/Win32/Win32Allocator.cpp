#include "stdjk/Mem/Allocator.h"
#include "stdjk/Mem/PageAllocator.h"
#include <Windows.h>

namespace mem {

Address AllocateAlign(USize Size, USize Alignment) {
    const USize align = Align(Size, Alignment);
    return VirtualAllocEx(
        GetCurrentProcess(),
        0, align,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
    );
}

void Deallocate(Address Mem) {
    VirtualFreeEx(GetCurrentProcess(), Mem, 0, MEM_RELEASE);
}

}
