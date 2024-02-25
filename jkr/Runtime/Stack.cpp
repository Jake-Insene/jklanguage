#include "jkr/Runtime/Stack.h"
#include "stdjk/Mem/Allocator.h"

namespace runtime {

Stack Stack::New(USize StackSize) {
    Value* elements = Cast<Value*>(
        mem::CountOf<Value>(StackSize)
    );

    return Stack{
        .Size = StackSize,
        .Start = elements,
    };
}

void Stack::Destroy(this Stack& Self) {
    mem::Deallocate(Self.Start);
}

}
