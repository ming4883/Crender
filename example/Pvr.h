#ifndef __EXAMPLE_PVR_H__
#define __EXAMPLE_PVR_H__

#include "../lib/crender/Platform.h"
#include "../lib/crender/Texture.h"

typedef unsigned long PvrDataType;

void Pvr_getTextureInfo(const PvrDataType* pvr, size_t* width, size_t* height, size_t* mipCount);

const void* Pvr_getTextureData(const PvrDataType* pvr, size_t* dataSizeInByte);

CrTexture* Pvr_createTexture(const PvrDataType* pvr);

#endif	// __EXAMPLE_PVR_H__