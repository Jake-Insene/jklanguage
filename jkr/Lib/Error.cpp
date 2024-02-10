#include "jkr/Lib/Error.h"

extern void __error_Exit(UInt Code);

namespace error {

void JK_API Exit(UInt Code) { 
    __error_Exit(Code);
    UNREACHABLE;
}

}
