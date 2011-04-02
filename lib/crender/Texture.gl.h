#ifndef __CRENDER_TEXTURE_GL_H__
#define __CRENDER_TEXTURE_GL_H__

#include "API.gl.h"
#include "Texture.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CrTextureGpuFormatMapping
{
	CrGpuFormat crFormat;
	size_t pixelSize;
	int internalFormat;
	int format;
	int type;
} CrTextureGpuFormatMapping;

typedef struct CrTextureImpl
{
	CrTexture i;
	int glTarget;
	GLuint glName;
	struct CrTextureGpuFormatMapping* apiFormatMapping;
} CrTextureImpl;

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_TEXTURE_H__
