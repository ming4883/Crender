#ifndef __CRENDER_STRHASH_H__
#define __CRENDER_STRHASH_H__

#include "Platform.h"
#include "StrHashMacro.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef size_t CrHashCode;
CrHashCode CrHashStruct(const void* data, size_t len);

#ifdef __cplusplus
}
#endif

#endif // __CRENDER_STRHASH_H__
