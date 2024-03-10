#include "jkr/NI/NI.h"
#include <Windows.h>

extern "C" JK_EXPORT JKValue write(JKThreadState State, JKUInt Fd, JKArray ArrayRef, JKUInt Count) {
    (void)State;
    HANDLE h = (HANDLE)Fd;
    if (Fd == 0) {
        h = GetStdHandle(STD_OUTPUT_HANDLE);
    }
    else if (Fd == 1) {
        h = GetStdHandle(STD_ERROR_HANDLE);
    }

    void* bytes = jkrArrayBytes(State, ArrayRef);
    WriteFile(h, bytes, (DWORD)Count, NULL, NULL);
    return JKValue{
        .U = 0,
    };
}

