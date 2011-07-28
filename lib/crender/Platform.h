#ifndef __CRENDER_PLATFORM_H__
#define __CRENDER_PLATFORM_H__

#if defined(_MSC_VER)
#	define CR_VC

#elif defined(__GNUC__)
#	define CR_GCC

#endif

#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER)

#	include <crtdbg.h>

#	if defined(CR_USE_DLL)
#		if(CR_DLL_EXPORT)
#			define CR_API __declspec(dllexport)
#		else
#			define CR_API __declspec(dllimport)
#		endif
#	else
#		define CR_API
#	endif
#else

#	define CR_API

#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef nullptr
#define nullptr 0
#endif

typedef unsigned char CrBool;
#define CrFalse 0
#define CrTrue 1

//! Plaform specific debug string output
CR_API void crDbgStr(const char* str, ...);

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
