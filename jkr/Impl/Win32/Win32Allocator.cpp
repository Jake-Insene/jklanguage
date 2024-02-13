#include "jkr/Mem/PageAllocator.h"
#include <Windows.h>
#include <stdio.h>

Address __mem__allocator_Allocate(USize Size) {
    return VirtualAllocEx(
        GetCurrentProcess(),
        0, Size,
        MEM_COMMIT | MEM_RESERVE,
        PAGE_READWRITE
    );
}

void __mem__allocator_Deallocate(Address Mem) {
    VirtualFreeEx(GetCurrentProcess(), Mem, 0, MEM_RELEASE);
}

