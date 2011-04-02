#ifndef __CRENDER_RENDERTARGET_H__
#define __CRENDER_RENDERTARGET_H__

#include "Platform.h"
#include "GpuFormat.h"

#ifdef __cplusplus
extern "C" {
#endif

struct CrTexture;
struct CrRenderBuffer;

typedef enum CrRenderTargetFlag
{
	CrRenderTarget_Inited = 1 << 0,
} CrRenderTargetFlag;

typedef struct CrRenderTarget
{
	size_t flags;

} CrRenderTarget;

typedef struct CrRenderBuffer
{
	struct CrTexture* texture;
} CrRenderBuffer;

CrRenderTarget* crRenderTargetAlloc();

void crRenderTargetFree(CrRenderTarget* self);

void crRenderTargetInit(CrRenderTarget* self);

CrRenderBuffer* crRenderTargetAcquireBuffer(CrRenderTarget* self, size_t width, size_t height, CrGpuFormat format);

void crRenderTargetReleaseBuffer(CrRenderTarget* self, CrRenderBuffer* buffer);

void crRenderTargetPreRender(CrRenderTarget* self, CrRenderBuffer** colors, CrRenderBuffer* depth);

void crRenderTargetSetViewport(float x, float y, float w, float h, float zmin, float zmax);

void crRenderTargetClearColor(float r, float g, float b, float a);

void crRenderTargetClearDepth(float z);

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_RENDERTARGET_H__