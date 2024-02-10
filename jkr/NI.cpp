#include "jkr/Runtime/Object.h"

extern "C" JK_EXPORT Address GetField(Address Object, unsigned short Index) {
    runtime::Object* obj = Cast<runtime::Object*>(Object);
    return Cast<Address>(
        &obj->Fields.Get(
            IntCast<USize>(Index)
        )
    );
}
