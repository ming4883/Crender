#ifndef __CRENDER_GPUSTATE_GL_H__
#define __CRENDER_GPUSTATE_GL_H__

#include "API.gl.h"
#include "GpuState.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CrGpuStateImpl
{
	CrGpuState i;
	CrGpuStateDesc last;
	
} CrGpuStateImpl;

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_GPUSTATE_GL_H__