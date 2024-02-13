#include "jkr/Lib/Error.h"
#include <Windows.h>

void __error_Exit(UInt Code) {
    ExitProcess(IntCast<UINT>(Code));
}
