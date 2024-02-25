#include "stdjk/Error.h"
#include <Windows.h>

namespace error {

void Exit(UInt Code) {
    ExitProcess(IntCast<UINT>(Code));
    UNREACHABLE;
}

}
