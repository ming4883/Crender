#ifndef __CRENDER_GPUSTATE_D3D9_H__
#define __CRENDER_GPUSTATE_D3D9_H__

#include "API.d3d9.h"
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

#endif	// __CRENDER_GPUSTATE_D3D9_H__