#include "Texture.gl.h"
#include "StrUtil.h"
#include "Mem.h"

#if defined(CR_GLES_2)
CrTextureGpuFormatMapping CrTextureGpuFormatMappings[] = {
	{CrGpuFormat_UnormR8G8B8A8, 4, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE},
	{CrGpuFormat_UnormR8, 1, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE},
	{CrGpuFormat_UnormA8, 1, GL_ALPHA, GL_ALPHA, GL_UNSIGNED_BYTE},
	{CrGpuFormat_UnormR5G5B5A1, 2, GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1},
	{CrGpuFormat_UnormR5G6B5, 2, GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5},
	{CrGpuFormat_UnormR4G4B4A4, 2, GL_RGBA, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4},
	{CrGpuFormat_CompPVRTC2, 2, GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, 0, 0},	// glCompressedTexImage2D does not need format & type
	{CrGpuFormat_CompPVRTC4, 1, GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, 0, 0},
	{CrGpuFormat_FloatR16, 2, GL_LUMINANCE, GL_LUMINANCE, GL_HALF_FLOAT_OES},
	{CrGpuFormat_FloatR32, 4, GL_LUMINANCE, GL_LUMINANCE, GL_FLOAT},
	{CrGpuFormat_FloatR16G16, 4, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_HALF_FLOAT_OES},
	{CrGpuFormat_FloatR32G32, 8, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_FLOAT},
	{CrGpuFormat_FloatR16G16B16A16, 8, GL_RGBA, GL_RGBA, GL_HALF_FLOAT_OES},
	{CrGpuFormat_FloatR32G32B32A32, 16, GL_RGBA, GL_RGBA, GL_FLOAT},
	{CrGpuFormat_Depth16, 2, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT},
};
#else
CrTextureGpuFormatMapping CrTextureGpuFormatMappings[] = {
	{CrGpuFormat_UnormR8G8B8A8, 4, GL_RGBA8, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV},
	{CrGpuFormat_UnormR8, 1, GL_R8, GL_RED, GL_UNSIGNED_BYTE},
	{CrGpuFormat_FloatR16, 2, GL_R16F, GL_RED, GL_HALF_FLOAT},
	{CrGpuFormat_FloatR32, 4, GL_R32F, GL_RED, GL_FLOAT},
	{CrGpuFormat_FloatR16G16, 4, GL_RG16F, GL_RG, GL_HALF_FLOAT},
	{CrGpuFormat_FloatR32G32, 8, GL_RG32F, GL_RG, GL_FLOAT},
	{CrGpuFormat_FloatR16G16B16A16, 8, GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT},
	{CrGpuFormat_FloatR32G32B32A32, 16, GL_RGBA32F, GL_RGBA, GL_FLOAT},
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
	CrTextureImpl* self = crMem()->alloc(sizeof(CrTextureImpl), "CrTexture");
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
			size_t mipSize = (*mipWidth) * (*mipHeight);

			if(CrGpuFormat_Compressed & self->format)
				mipSize = mipSize >> impl->apiFormatMapping->pixelSize;
			else
				mipSize = mipSize * impl->apiFormatMapping->pixelSize;

			offset += mipSize;
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
		crDbgStr("Non supported texture format: %08x\n", format);
		return CrFalse;
	}

	crCheckGLError();	// clear any unhandled gl errors

	self->format = format;
	self->width = width;
	self->height = height;
	self->mipCount = mipCount;
	self->surfCount = surfCount;

	{
		size_t tmpw, tmph;
		self->surfSizeInByte = crTextureGetMipLevelOffset(self, self->mipCount+1, &tmpw, &tmph);
	}

	glGenTextures(1, &impl->glName);

	if(self->surfCount == 1) {
		impl->glTarget = GL_TEXTURE_2D;
		glBindTexture(impl->glTarget, impl->glName);
		//glTexParameteri(impl->glTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(impl->glTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	self->flags = CrTexture_Inited;

	return crTextureCommit(self, data);
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
		crDbgStr("Non supported texture format: %08x\n", format);
		return CrFalse;
	}

	crCheckGLError();	// clear any unhandled gl errors

	self->format = format;
	self->width = width;
	self->height = height;
	self->mipCount = mipCount;
	self->surfCount = surfCount;

	{ size_t tmpw, tmph;
	self->surfSizeInByte = crTextureGetMipLevelOffset(self, self->mipCount+1, &tmpw, &tmph);}

	if((self->format & CrGpuFormat_Depth) && !crContextDepthTextureSupport()) {
		glGenRenderbuffers(1, &impl->glName);
		glBindRenderbuffer(GL_RENDERBUFFER, impl->glName);
		
		self->flags = CrTexture_Inited | CrTexture_RenderTarget;

		return crTextureCommit(self, nullptr);
	}
	else {
		glGenTextures(1, &impl->glName);

		if(self->surfCount == 1) {
			impl->glTarget = GL_TEXTURE_2D;
			glBindTexture(impl->glTarget, impl->glName);
		}

		self->flags = CrTexture_Inited | CrTexture_RenderTarget;

		return crTextureCommit(self, nullptr);
	}
}


CR_API unsigned char* crTextureGetMipLevel(CrTexture* self, unsigned char* data, size_t surfIndex, size_t mipIndex, size_t* mipWidth, size_t* mipHeight)
{
	if(nullptr == self)
		return nullptr;

	if(surfIndex >= self->surfCount)
		return nullptr;

	if(mipIndex > self->mipCount)
		return nullptr;

	return data + (surfIndex * self->surfSizeInByte) + crTextureGetMipLevelOffset(self, mipIndex, mipWidth, mipHeight);
}

CR_API CrBool crTextureCommit(CrTexture* self, const void* data)
{
	const CrTextureGpuFormatMapping* mapping;
	CrTextureImpl* impl = (CrTextureImpl*)self;

	if(nullptr == self)
		return CrFalse;

	if(nullptr == impl->apiFormatMapping)
		return CrFalse;

	crCheckGLError();	// clear any unhandled gl errors

	mapping = impl->apiFormatMapping;

	if((self->format & CrGpuFormat_Depth) && !crContextDepthTextureSupport()) {
		glRenderbufferStorage(GL_RENDERBUFFER, mapping->internalFormat, self->width, self->height);
		return CrTrue;
	}

	if(self->surfCount == 1) {

		size_t i;

		glBindTexture(impl->glTarget, impl->glName);

		for(i=0; i<self->mipCount+1; ++i) {
			size_t mipW, mipH;

			unsigned char* mipdata = crTextureGetMipLevel(self, (unsigned char*)data, 0, i, &mipW, &mipH);

			if(CrGpuFormat_Compressed &mapping->crFormat) {
				size_t imgSize = (mipW * mipH) >> mapping->pixelSize;
				glCompressedTexImage2D(impl->glTarget, i, mapping->internalFormat, 
					mipW, mipH, 0,
					imgSize,
					nullptr == data ? nullptr : mipdata);
			}
			else {
				glTexImage2D(impl->glTarget, i, mapping->internalFormat,
					mipW, mipH, 0,
					mapping->format, mapping->type,
					nullptr == data ? nullptr : mipdata);
			}
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

	if(0 != impl->glName) {
		if((self->format & CrGpuFormat_Depth) && !crContextDepthTextureSupport())
			glDeleteRenderbuffers(1, &impl->glName);
		else
			glDeleteTextures(1, &impl->glName);
	}

	crMem()->free(self, "CrTexture");
}
