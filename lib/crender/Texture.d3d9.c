#include "Texture.d3d9.h"
#include "StrUtil.h"
#include "Memory.h"

CrTextureGpuFormatMapping CrTextureGpuFormatMappings[] = {
	{CrGpuFormat_UnormR8G8B8A8, 4, D3DFMT_A8R8G8B8},
	{CrGpuFormat_UnormR8, 1, D3DFMT_L8},
	{CrGpuFormat_FloatR16, 2, D3DFMT_R16F},
	{CrGpuFormat_FloatR32, 4, D3DFMT_R32F},
	{CrGpuFormat_FloatR16G16B16A16, 8, D3DFMT_A16B16G16R16F},
	{CrGpuFormat_FloatR32G32B32A32, 16, D3DFMT_A32B32G32R32F},
};

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

CrTexture* crTextureAlloc()
{
	CrTextureImpl* self = crMemory()->alloc(sizeof(CrTextureImpl), "CrTexture");
	memset(self, 0, sizeof(CrTextureImpl));
	return &self->i;
}

size_t CrGpuFormat_getMipLevelOffset(CrTexture* self, size_t mipIndex, size_t* mipWidth, size_t* mipHeight)
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

void crTextureInit(CrTexture* self, size_t width, size_t height, size_t mipCount, size_t surfCount, CrGpuFormat format)
{
	CrTextureImpl* impl = (CrTextureImpl*)self;

	if(self->flags & CrTextureFlag_Inited) {
		crDbgStr("texture already inited!\n");
		return;
	}

	if(surfCount > 1) {
		crDbgStr("Current not support surfCount > 1!\n");
		return;
	}

	impl->apiFormatMapping = crTextureGpuFormatMappingGet(format);
	
	if(nullptr == impl->apiFormatMapping) {
		crDbgStr("Non supported texture format: %s\n", format);
		return;
	}

	self->format = format;
	self->width = width;
	self->height = height;
	self->mipCount = mipCount;
	self->surfCount = surfCount;

	// init cache memory
	{
		size_t tmpw, tmph;
		self->surfSizeInByte = CrGpuFormat_getMipLevelOffset(self, self->mipCount+1, &tmpw, &tmph);
		self->data = (unsigned char*)crMemory()->alloc(self->surfSizeInByte * self->surfCount, "CrTexture");
		memset(self->data, 0, self->surfSizeInByte * self->surfCount);
	}

	// create texture
	{
		HRESULT hr;
		hr = IDirect3DDevice9_CreateTexture(crAPI.d3ddev, width, height, self->mipCount, 0, impl->apiFormatMapping->d3dFormat, D3DPOOL_DEFAULT, &impl->d3dtex, nullptr);
		if(FAILED(hr)) {
			crDbgStr("d3d9 failed to create texture %8x", hr);
			return;
		}
	}

	crTextureCommit(self);
}

void crTextureInitRtt(CrTexture* self, size_t width, size_t height, size_t mipCount, size_t surfCount, CrGpuFormat format)
{
	CrTextureImpl* impl = (CrTextureImpl*)self;

	if(self->flags & CrTextureFlag_Inited) {
		crDbgStr("texture already inited!\n");
		return;
	}

	if(surfCount > 1) {
		crDbgStr("Current not support surfCount > 1!\n");
		return;
	}

	impl->apiFormatMapping = crTextureGpuFormatMappingGet(format);
	
	if(nullptr == impl->apiFormatMapping) {
		crDbgStr("Non supported texture format: %s\n", format);
		return;
	}

	self->format = format;
	self->width = width;
	self->height = height;
	self->mipCount = mipCount;
	self->surfCount = surfCount;

	// set cache memory to null
	{
		size_t tmpw, tmph;
		self->surfSizeInByte = CrGpuFormat_getMipLevelOffset(self, self->mipCount+1, &tmpw, &tmph);
		self->data = nullptr;
	}

	// create d3d texture
	{
		HRESULT hr;
		hr = IDirect3DDevice9_CreateTexture(crAPI.d3ddev, width, height, self->mipCount, D3DUSAGE_RENDERTARGET, impl->apiFormatMapping->d3dFormat, D3DPOOL_DEFAULT, &impl->d3dtex, nullptr);
		if(FAILED(hr)) {
			crDbgStr("d3d9 failed to create texture %8x", hr);
			return;
		}
	}
}


unsigned char* crTextureGetMipLevel(CrTexture* self, size_t surfIndex, size_t mipIndex, size_t* mipWidth, size_t* mipHeight)
{
	if(nullptr == self)
		return nullptr;

	if(surfIndex >= self->surfCount)
		return nullptr;

	if(mipIndex > self->mipCount)
		return nullptr;

	//if(nullptr == self->data)
	//	return nullptr;

	return self->data + (surfIndex * self->surfSizeInByte) + CrGpuFormat_getMipLevelOffset(self, mipIndex, mipWidth, mipHeight);
}

void crTextureCommit(CrTexture* self)
{
	CrTextureImpl* impl = (CrTextureImpl*)self;

	if(nullptr == self)
		return;

	if(nullptr == impl->apiFormatMapping)
		return;

	if(self->surfCount == 1) {
		HRESULT hr;
		size_t i;
		IDirect3DTexture9* stageTex;

		hr = IDirect3DDevice9_CreateTexture(crAPI.d3ddev, self->width, self->height, self->mipCount, 0, impl->apiFormatMapping->d3dFormat, D3DPOOL_SYSTEMMEM, &stageTex, nullptr);
		if(FAILED(hr)) {
			crDbgStr("d3d9 failed to create texture %8x", hr);
			return;
		}

		for(i=0; i<self->mipCount; ++i) {
			size_t mipW, mipH;
			D3DLOCKED_RECT locked;
			unsigned char* data = crTextureGetMipLevel(self, 0, i, &mipW, &mipH);

			hr = IDirect3DTexture9_LockRect(stageTex, i, &locked, nullptr, 0);
			if(FAILED(hr))
				continue;

			memcpy(locked.pBits, data, mipW * mipH * impl->apiFormatMapping->pixelSize);
			IDirect3DTexture9_UnlockRect(stageTex, i);
		}

		IDirect3DDevice9_UpdateTexture(crAPI.d3ddev, (IDirect3DBaseTexture9*)stageTex, (IDirect3DBaseTexture9*)impl->d3dtex);

		IDirect3DTexture9_Release(stageTex);
	}
}

void crTextureFree(CrTexture* self)
{
	CrTextureImpl* impl = (CrTextureImpl*)self;

	if(nullptr == self)
		return;

	if(nullptr != self->data)
		crMemory()->free(self->data, "CrTexture");

	if(nullptr != impl->d3dtex) {
		IDirect3DTexture9_Release(impl->d3dtex);
	}
	
	crMemory()->free(self, "CrTexture");
}