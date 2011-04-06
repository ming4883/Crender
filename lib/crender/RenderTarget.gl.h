#ifndef __CRENDER_RENDERTARGET_GL_H__
#define __CRENDER_RENDERTARGET_GL_H__

#include "Context.gl.h"
#include "RenderTarget.h"
#include "Texture.gl.h"
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

} CrRenderBufferImpl;

typedef struct CrRenderTargetImpl
{
	CrRenderTarget i;

	GLuint glName;
	size_t bufferCount;
	struct CrRenderBufferImpl* bufferList;
} CrRenderTargetImpl;


#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_RENDERTARGET_GL_H__
