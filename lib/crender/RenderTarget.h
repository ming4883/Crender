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

CR_API CrRenderTarget* crRenderTargetAlloc();

CR_API void crRenderTargetFree(CrRenderTarget* self);

CR_API void crRenderTargetInit(CrRenderTarget* self);

CR_API CrRenderBuffer* crRenderTargetAcquireBuffer(CrRenderTarget* self, size_t width, size_t height, CrGpuFormat format);

CR_API void crRenderTargetReleaseBuffer(CrRenderTarget* self, CrRenderBuffer* buffer);

CR_API void crRenderTargetPreRender(CrRenderTarget* self, CrRenderBuffer** colors, CrRenderBuffer* depth);

CR_API void crRenderTargetSetViewport(float x, float y, float w, float h, float zmin, float zmax);

CR_API void crRenderTargetClearColor(float r, float g, float b, float a);

CR_API void crRenderTargetClearDepth(float z);

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_RENDERTARGET_H__
