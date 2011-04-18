#include "Texture.gl.h"
#include "StrUtil.h"
#include "Memory.h"

#if defined(CR_GLES_2)
CrTextureGpuFormatMapping CrTextureGpuFormatMappings[] = {
	{CrGpuFormat_UnormR8G8B8A8, 4, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE},
	{CrGpuFormat_UnormR8, 1, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE},
	{CrGpuFormat_UnormR5G5B5A1, 2, GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1},
	{CrGpuFormat_UnormR5G6B5, 2, GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5},
	{CrGpuFormat_UnormR4G4B4A4, 2, GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4},
};
#else
CrTextureGpuFormatMapping CrTextureGpuFormatMappings[] = {
	{CrGpuFormat_UnormR8G8B8A8, 4, GL_RGBA8, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV},
	{CrGpuFormat_UnormR8, 1, GL_R8, GL_RED, GL_UNSIGNED_BYTE},
	{CrGpuFormat_FloatR16, 2, GL_R16F, GL_RED, GL_HALF_FLOAT},
	{CrGpuFormat_FloatR32, 4, GL_R32F, GL_RED, GL_FLOAT},
	{CrGpuFormat_FloatR16G16B16A16, 8, GL_RGBA16F, GL_BGRA, GL_HALF_FLOAT},
	{CrGpuFormat_FloatR32G32B32A32, 16, GL_RGBA32F, GL_BGRA, GL_FLOAT},
	{CrGpuFormat_Depth16, 2, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT},  
	{CrGpuFormat_Depth32, 4, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT},
};
#endif

CrTextureGpuFormatMapping* crTextureGpuFormatMappingGet(CrGpuFormat crFormat)
{
	size_t i=0;
	for(i=0; i<crCountOf(CrTextureGpuFormatMappings); ++i) {
		CrTextureGpuFormatMapping* mapping = &CrTextureGpuFormatMappings[i];
		if(crFormat == mapping->crFormat)
			return mapping;
	}

	return nullptr;
}

CR_API CrTexture* crTextureAlloc()
{
	CrTextureImpl* self = crMemory()->alloc(sizeof(CrTextureImpl), "CrTexture");
	memset(self, 0, sizeof(CrTextureImpl));
	return &self->i;
}

size_t crTextureGetMipLevelOffset(CrTexture* self, size_t mipIndex, size_t* mipWidth, size_t* mipHeight)
{
	size_t i = 0;
	size_t offset = 0;
	CrTextureImpl* impl = (CrTextureImpl*)self;
	
	*mipWidth = self->width;
	*mipHeight = self->height;
	
	do {
		if(i < mipIndex) {
			offset += impl->apiFormatMapping->pixelSize * (*mipWidth) * (*mipHeight);
			if(*mipWidth > 1) *mipWidth /= 2;
			if(*mipHeight > 1) *mipHeight /= 2;
		}
	} while(++i < mipIndex);

	return offset;
}

CR_API CrBool crTextureInit(CrTexture* self, size_t width, size_t height, size_t mipCount, size_t surfCount, CrGpuFormat format, const void* data)
{
	CrTextureImpl* impl = (CrTextureImpl*)self;
	if(self->flags & CrTexture_Inited) {
		crDbgStr("texture already inited!\n");
		return CrFalse;
	}

	if(surfCount > 1) {
		crDbgStr("Current not support surfCount > 1!\n");
		return CrFalse;
	}

	impl->apiFormatMapping = crTextureGpuFormatMappingGet(format);
	
	if(nullptr == impl->apiFormatMapping) {
		crDbgStr("Non supported texture format: %s\n", format);
		return CrFalse;
	}

	self->format = format;
	self->width = width;
	self->height = height;
	self->mipCount = mipCount;
	self->surfCount = surfCount;

	{
		size_t tmpw, tmph;
		self->surfSizeInByte = crTextureGetMipLevelOffset(self, self->mipCount+1, &tmpw, &tmph);
		self->data = (unsigned char*)crMemory()->alloc(self->surfSizeInByte * self->surfCount, "CrTexture");

		if(nullptr != data)
			memcpy(self->data, data, self->surfSizeInByte * self->surfCount);
		else
			memset(self->data, 0, self->surfSizeInByte * self->surfCount);
	}

	glGenTextures(1, &impl->glName);
	
	if(self->surfCount == 1) {
		impl->glTarget = GL_TEXTURE_2D;
		glBindTexture(impl->glTarget, impl->glName);
		//glTexParameteri(impl->glTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(impl->glTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	return crTextureCommit(self);
}

CR_API CrBool crTextureInitRtt(CrTexture* self, size_t width, size_t height, size_t mipCount, size_t surfCount, CrGpuFormat format)
{
	CrTextureImpl* impl = (CrTextureImpl*)self;

	if(self->flags & CrTexture_Inited) {
		crDbgStr("texture already inited!\n");
		return CrFalse;
	}

	if(surfCount > 1) {
		crDbgStr("Current not support surfCount > 1!\n");
		return CrFalse;
	}

	impl->apiFormatMapping = crTextureGpuFormatMappingGet(format);
	
	if(nullptr == impl->apiFormatMapping) {
		crDbgStr("Non supported texture format: %s\n", format);
		return CrFalse;
	}

	self->format = format;
	self->width = width;
	self->height = height;
	self->mipCount = mipCount;
	self->surfCount = surfCount;

	{
		size_t tmpw, tmph;
		self->surfSizeInByte = crTextureGetMipLevelOffset(self, self->mipCount+1, &tmpw, &tmph);
		self->data = nullptr;
	}

	glGenTextures(1, &impl->glName);
	
	if(self->surfCount == 1) {
		impl->glTarget = GL_TEXTURE_2D;
		glBindTexture(impl->glTarget, impl->glName);
		//glTexParameteri(impl->glTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(impl->glTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	return crTextureCommit(self);
}


CR_API unsigned char* crTextureGetMipLevel(CrTexture* self, size_t surfIndex, size_t mipIndex, size_t* mipWidth, size_t* mipHeight)
{
	if(nullptr == self)
		return nullptr;

	if(surfIndex >= self->surfCount)
		return nullptr;

	if(mipIndex > self->mipCount)
		return nullptr;

	//if(nullptr == self->data)
	//	return nullptr;

	return self->data + (surfIndex * self->surfSizeInByte) + crTextureGetMipLevelOffset(self, mipIndex, mipWidth, mipHeight);
}

CR_API CrBool crTextureCommit(CrTexture* self)
{
	const CrTextureGpuFormatMapping* mapping;
	CrTextureImpl* impl = (CrTextureImpl*)self;

	if(nullptr == self)
		return CrFalse;

	if(nullptr == impl->apiFormatMapping)
		return CrFalse;
	
	mapping = impl->apiFormatMapping;

	if(self->surfCount == 1) {
		
		size_t i;

		glBindTexture(impl->glTarget, impl->glName);
		//glTexImage2D(impl->glTarget, 0, mapping->internalFormat, self->width, self->height, 0, mapping->format, mapping->type, self->data);
		
		for(i=0; i<self->mipCount+1; ++i) {
			size_t mipW, mipH;
			unsigned char* data = crTextureGetMipLevel(self, 0, i, &mipW, &mipH);
			glTexImage2D(impl->glTarget, i, mapping->internalFormat, mipW, mipH, 0, mapping->format, mapping->type, data);
		}
	}

	{
		GLenum err = glGetError();

		if(GL_NO_ERROR != err) {
			crDbgStr("failed to commit texture: 0x%04x\n", (int)err);
			return CrFalse;
		}
	}

	return CrTrue;
}

CR_API void crTextureFree(CrTexture* self)
{
	CrTextureImpl* impl = (CrTextureImpl*)self;

	if(nullptr == self)
		return;

	if(nullptr != self->data)
		crMemory()->free(self->data, "CrTexture");

	if(0 != impl->glName)
		glDeleteTextures(1, &impl->glName);
	
	crMemory()->free(self, "CrTexture");
}
