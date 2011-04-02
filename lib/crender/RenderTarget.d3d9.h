#ifndef __CRENDER_RENDERTARGET_D3D9_H__
#define __CRENDER_RENDERTARGET_D3D9_H__

#include "API.d3d9.h"
#include "RenderTarget.h"
#include "Texture.d3d9.h"
#include "uthash/utlist.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CrRenderBufferImpl
{
	CrRenderBuffer i;

	struct CrRenderBufferImpl* next;
	struct CrRenderBufferImpl* last;
	CrBool acquired;
	IDirect3DSurface9* d3dsurf;

} CrRenderBufferImpl;

typedef struct CrRenderTargetImpl
{
	CrRenderTarget i;

	size_t bufferCount;
	struct CrRenderBufferImpl* bufferList;
} CrRenderTargetImpl;


#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_RENDERTARGET_D3D9_H__