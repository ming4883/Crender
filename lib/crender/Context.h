#ifndef __CRENDER_CONTEXT_H__
#define __CRENDER_CONTEXT_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CrContext
{
	const char* apiName;
	size_t apiMajorVer;
	size_t apiMinorVer;
	size_t xres;
	size_t yres;
	size_t msaaLevel;
	CrBool vsync;

} CrContext;

CR_API CrContext* crContext();

CR_API CrContext* crContextAlloc();

CR_API CrBool crContextInit(CrContext* self, void** window);

CR_API void crContextFree(CrContext* self);

CR_API CrBool crContextPreRender(CrContext* self);

CR_API void crContextPostRender(CrContext* self);

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_CONTEXT_H__
