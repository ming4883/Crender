//----------------------------------------------------------------------------------
// File:            libs\jni\nv_types.h
// Samples Version: Android NVIDIA samples 2 
// Email:           tegradev@nvidia.com
// Forum:           http://developer.nvidia.com/tegra/forums/tegra-forums/android-development
//
// Copyright 2009-2010 NVIDIA® Corporation 
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//----------------------------------------------------------------------------------
#ifndef __INCLUDED_NV_TYPES_H
#define __INCLUDED_NV_TYPES_H

typedef unsigned char      NvU8;  // 0 to 255
typedef unsigned short     NvU16; // 0 to 65535
typedef unsigned int       NvU32; // 0 to 4294967295
typedef unsigned long long NvU64; // 0 to 18446744073709551615
typedef signed char        NvS8;  // -128 to 127
typedef signed short       NvS16; // -32768 to 32767
typedef signed int         NvS32; // -2147483648 to 2147483647
typedef signed long long   NvS64; // 2^-63 to 2^63-1

// Explicitly sized floats
typedef float              NvF32; // IEEE Single Precision (S1E8M23)
typedef double             NvF64; // IEEE Double Precision (S1E11M52)

// Boolean type
#define NV_FALSE    0
#define NV_TRUE     1
typedef NvU8 NvBool;

// Result of sizeof operator 
typedef unsigned long NvSize;

// Type large enough to hold a relative file offset
typedef long NvOffset;

// Base NULL type.
#define NV_NULL 0

// Error related
typedef enum {
    NvError_Success = 0,
    NvError_NotSupported,
    NvError_NotInitialized,
    NvError_BadParameter,
    NvError_InsufficientMemory,
    NvError_NoEntries,
    NvError_UnknownError,
} NvError;

#if 1 //defined(_DEBUG) // !!!!TBD TODO
#define NvAssert(c)
#else
#define NvAssert(c) ((void)( (c) ? 0 : (NvHandleAssertion(#c, __FILE__, __LINE__), 0)))
#endif


// Other standardized typedefs
typedef NvU64               NvUST; // U64 unadjusted system time value


#endif
