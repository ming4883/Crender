#ifndef __CRENDER_PLATFORM_H__
#define __CRENDER_PLATFORM_H__

#if defined(_MSC_VER)
#	define CR_VC

#elif defined(__GNUC__)
#	define CR_GCC

#endif

#include <stdlib.h>
#include <string.h>

#ifdef CR_APPLE_IOS
#include <stddef.h>
#include <stdint.h>
#else
#include "pstdint.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define nullptr 0

typedef uint8_t CrBool;
#define CrFalse 0
#define CrTrue 1

void crDbgStr(const char* str, ...);

#define crCountOf(A) (sizeof(A) / sizeof(A[0]))

#define crMin(a, b) (a < b ? a : b)

#define crMax(a, b) (a > b ? a : b)

#define CrAllocWithImpl(obj, CLASS, CLASSIMPL) \
	obj = (CLASS*)malloc(sizeof(CLASS)+sizeof(CLASSIMPL));\
	memset(obj, 0, sizeof(CLASS)+sizeof(CLASSIMPL));\
	obj->impl = (CLASSIMPL*)((char*)obj + sizeof(CLASS));

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_PLATFORM_H__
