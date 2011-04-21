#include "RenderTarget.d3d9.h"
#include "Mem.h"

CR_API CrRenderTarget* crRenderTargetAlloc()
{
	CrRenderTargetImpl* self = crMem()->alloc(sizeof(CrRenderTargetImpl), "CrRenderTarget");
	memset(self, 0, sizeof(CrRenderTargetImpl));
	return &self->i;
}

CR_API void crRenderTargetFree(CrRenderTarget* self)
{
	CrRenderTargetImpl* impl = (CrRenderTargetImpl*)self;

	if(self->flags & CrRenderTarget_Inited) {
		
		CrRenderBufferImpl* it; CrRenderBufferImpl* tmp;

		LL_FOREACH_SAFE(impl->bufferList, it, tmp) {
			LL_DELETE(impl->bufferList, it);
			crTextureFree(it->i.texture);
			IDirect3DSurface9_Release(it->d3dsurf);
			crMem()->free(it, "CrRenderTarget");
		}
	}
	crMem()->free(self, "CrRenderTarget");
}

CR_API void crRenderTargetInit(CrRenderTarget* self)
{
	CrRenderTargetImpl* impl = (CrRenderTargetImpl*)self;

	if(nullptr == self)
		return;

	if(self->flags & CrRenderTarget_Inited) {
		crDbgStr("CrRenderTarget already inited!\n");
		return;
	}

	self->flags |= CrRenderTarget_Inited;
}

D3DFORMAT crD3D9_DEPTH_FORMAT[] = {
	D3DFMT_D16,
	D3DFMT_D32,
	D3DFMT_D24S8,
};

CR_API CrRenderBuffer* crRenderTargetAcquireBuffer(CrRenderTarget* self, size_t width, size_t height, CrGpuFormat format)
{
	CrRenderTargetImpl* impl = (CrRenderTargetImpl*)self;

	CrRenderBufferImpl* buffer;
	CrRenderBufferImpl* it;
	LL_FOREACH(impl->bufferList, it) {
		CrTexture* tex = it->i.texture;
		if(CrFalse == it->acquired && (width == tex->width) && (height == tex->height) && (format == tex->format)) {
			return &it->i;
		}
	}

	buffer = crMem()->alloc(sizeof(CrRenderBufferImpl), "CrRenderTarget");
	memset(buffer, 0, sizeof(CrRenderBufferImpl));
	buffer->acquired = CrTrue;

	if(format & CrGpuFormat_Depth) {
		IDirect3DDevice9_CreateDepthStencilSurface(
			crContextImpl()->d3ddev,
			width, height,
			crD3D9_DEPTH_FORMAT[CrGpuFormat_Depth & 0x0000ffff],
			D3DMULTISAMPLE_NONE, 0,
			TRUE, 
			&buffer->d3dsurf,
			nullptr);
	}
	else {
		buffer->i.texture = crTextureAlloc();
		crTextureInitRtt(buffer->i.texture, width, height, 0, 1, format);
		IDirect3DTexture9_GetSurfaceLevel(
			((CrTextureImpl*)buffer->i.texture)->d3dtex, 0,
			&buffer->d3dsurf);
	}

	LL_APPEND(impl->bufferList, buffer);
	++impl->bufferCount;

	return &buffer->i;
}

CR_API void crRenderTargetReleaseBuffer(CrRenderTarget* self, CrRenderBuffer* buffer)
{
	if(nullptr == self)
		return;

	if(nullptr == buffer)
		return;

	((CrRenderBufferImpl*)buffer)->acquired = CrFalse;
}

CR_API void crRenderTargetPreRender(CrRenderTarget* self, CrRenderBuffer** colors, CrRenderBuffer* depth)
{
	CrRenderTargetImpl* impl = (CrRenderTargetImpl*)self;

	size_t bufCnt;
	CrRenderBuffer** curr;
	if(nullptr == self) {
		IDirect3DDevice9_SetRenderTarget(crContextImpl()->d3ddev, 0, crContextImpl()->d3dcolorbuf);
		IDirect3DDevice9_SetDepthStencilSurface(crContextImpl()->d3ddev, crContextImpl()->d3ddepthbuf);
		return;
	}

	// attach color buffers
	bufCnt = 0;
	if(nullptr != colors) {
		curr = (CrRenderBuffer**)colors;
		while(*curr != nullptr) {
			CrRenderBufferImpl* buf = (CrRenderBufferImpl*)*curr;
			IDirect3DDevice9_SetRenderTarget(crContextImpl()->d3ddev, bufCnt, buf->d3dsurf);
			++curr;
			++bufCnt;
		}
	}

	// attach depth buffers
	if(depth != nullptr) {
		CrTexture* tex = depth->texture;
		CrRenderBufferImpl* buf = (CrRenderBufferImpl*)depth;
		IDirect3DDevice9_SetDepthStencilSurface(crContextImpl()->d3ddev, buf->d3dsurf);
	}
	else {
		IDirect3DDevice9_SetDepthStencilSurface(crContextImpl()->d3ddev, nullptr);
	}
}

CR_API void crRenderTargetSetViewport(float x, float y, float w, float h, float zmin, float zmax)
{
	D3DVIEWPORT9 vp;
	vp.X = (DWORD)x;
	vp.Y = (DWORD)y;
	vp.Width  = (DWORD)w;
	vp.Height = (DWORD)h;
	vp.MinZ = zmin * 0.5f + 0.5f;
	vp.MaxZ = zmax * 0.5f + 0.5f;;

	IDirect3DDevice9_SetViewport(crContextImpl()->d3ddev, &vp);
}

CR_API void crRenderTargetClearColor(float r, float g, float b, float a)
{
	IDirect3DDevice9_Clear(crContextImpl()->d3ddev, 0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_COLORVALUE(r, g, b, a), 1, 0);
}

CR_API void crRenderTargetClearDepth(float z)
{
	IDirect3DDevice9_Clear(crContextImpl()->d3ddev, 0, nullptr, D3DCLEAR_ZBUFFER, 0, z, 0);
}