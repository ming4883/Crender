#ifndef __CRENDER_API_D3D9_H__
#define __CRENDER_API_D3D9_H__

#include <d3d9.h>
#include <d3dx9.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CrAPI
{
	IDirect3D9* d3d;
	IDirect3DDevice9* d3ddev;
	IDirect3DSurface9* d3dcolorbuf;
	IDirect3DSurface9* d3ddepthbuf;
	unsigned int gpuInputId;
} CrAPI;

extern CrAPI crAPI;

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_API_D3D9_H__