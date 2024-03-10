#pragma once

namespace codefile {

struct DataHeader {
    Byte Primitive;
    // If Attributes has TypeArray then
    //  Array Size [32 bits]
    //  Primitive Data[ArraySize];
    // Else
    //      Data based on primitive...
};

}
