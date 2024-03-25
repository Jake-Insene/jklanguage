#include "jkr/Runtime/Stack.h"
#include "jkr/Definitions.h"
#include "jkr/Align.h"

namespace runtime {

Stack::Stack(USize StackSize) :
    Size(StackSize) {
    Start = new Value[StackSize]{};
}

Stack::~Stack() {
    delete[] Start;
}

}
