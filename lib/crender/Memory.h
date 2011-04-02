#ifndef __CRENDER_MEMORY_H__
#define __CRENDER_MEMORY_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void*(* CrAllocFunc) (size_t sizeInBytes, const char* id);
typedef void(* CrFreeFunc) (void* ptr, const char* id);

typedef struct CrMemory
{
	CrAllocFunc alloc;
	CrFreeFunc free;
} CrMemory;

CrMemory* crMemory();

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_MEMORY_H__