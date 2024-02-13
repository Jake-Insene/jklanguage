#pragma once

using Int8 = char;
using Uint8 = unsigned char;
using Int16 = short;
using Uint16 = unsigned short;
using Int32 = int;
using Uint32 = unsigned int;
using Int64 = long long;
using Uint64 = unsigned long long;

using Float32 = float;
using Float64 = double;

using Byte = unsigned char;
using Char = char8_t;
using Int = long long;
using UInt = unsigned long long;
using Float = double;

using Str = const Char*;

#if defined(_M_AMD64) || defined(_M_X64)
using IntPtr = UInt;
using ISize = Int;
using USize = UInt;
#else
using IntPtr = unsigned int;
using ISize = int;
using USize = unsigned int;
#endif

using Address = void*;
