#pragma once
#include "stdjk/CoreHeader.h"

namespace runtime {
struct VirtualMachine;
}

struct ThreadState {
    runtime::VirtualMachine* VM;
};
