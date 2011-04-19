#ifndef __CRENDER_BUFFER_D3D9_H__
#define __CRENDER_BUFFER_D3D9_H__

#include "Context.d3d9.h"
#include "Buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CrBufferImpl
{
	CrBuffer i;

	IDirect3DVertexBuffer9* d3dvb;
	IDirect3DIndexBuffer9* d3dib;
	
} CrBufferImpl;

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_BUFFER_D3D9_H__