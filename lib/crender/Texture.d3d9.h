#ifndef __CRENDER_TEXTURE_D3D9_H__
#define __CRENDER_TEXTURE_D3D9_H__

#include "Context.d3d9.h"
#include "Texture.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CrTextureGpuFormatMapping
{
	CrGpuFormat crFormat;
	size_t pixelSize;
	D3DFORMAT d3dFormat;
} CrTextureGpuFormatMapping;

typedef struct CrTextureImpl
{
	CrTexture i;
	IDirect3DTexture9* d3dtex;
	IDirect3DSurface9* d3dsurf;
	struct CrTextureGpuFormatMapping* apiFormatMapping;
} CrTextureImpl;

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_TEXTURE_D3D9_H__