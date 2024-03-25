#include "jkr/NI/NI.h"
#include <Windows.h>

extern "C" JK_EXPORT void write(JKUInt Fd, JKArray ArrayRef, JKUInt Count) {
    HANDLE h = (HANDLE)Fd;
    if (Fd == 0) {
        h = GetStdHandle(STD_OUTPUT_HANDLE);
    }
    else if (Fd == 1) {
        h = GetStdHandle(STD_ERROR_HANDLE);
    }

    void* bytes = jkrArrayBytes(ArrayRef);
    WriteFile(h, bytes, (DWORD)Count, NULL, NULL);
}

