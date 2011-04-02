#ifndef __CRENDER_TEXTURE_H__
#define __CRENDER_TEXTURE_H__

#include "Platform.h"
#include "GpuFormat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum CrTextureFlag
{
	CrTextureFlag_Inited = 1 << 0,
	CrTextureFlag_Dirty = 1 << 1,
} CrTextureFlag;

typedef struct CrTexture
{
	size_t flags;
	size_t width;
	size_t height;
	size_t mipCount;	//<! include the base level
	size_t surfCount;
	size_t surfSizeInByte;
	CrGpuFormat format;
	unsigned char* data;	//<! nullptr implies this is a render-target
} CrTexture;

CrTexture* crTextureAlloc();

void crTextureFree(CrTexture* self);

void crTextureInit(CrTexture* self, size_t width, size_t height, size_t mipCount, size_t surfCount, CrGpuFormat format);

void crTextureInitRtt(CrTexture* self, size_t width, size_t height, size_t mipCount, size_t surfCount, CrGpuFormat format);

//! mipIndex zero-base mip level index
unsigned char* crTextureGetMipLevel(CrTexture* self, size_t surfIndex, size_t mipIndex, size_t* mipWidth, size_t* mipHeight);

void crTextureCommit(CrTexture* self);

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_TEXTURE_H__