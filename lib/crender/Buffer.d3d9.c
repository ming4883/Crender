#include "Buffer.d3d9.h"
#include "Mem.h"

CR_API CrBuffer* crBufferAlloc()
{
	CrBufferImpl* self = crMem()->alloc(sizeof(CrBufferImpl), "CrBuffer");
	memset(self, 0, sizeof(CrBufferImpl));
	return &self->i;
}

CR_API void crBufferFree(CrBuffer* self)
{
	CrBufferImpl* impl = (CrBufferImpl*)self;

	if(nullptr == self)
		return;

	if(nullptr != impl->d3dvb)
		IDirect3DVertexBuffer9_Release(impl->d3dvb);

	if(nullptr != impl->d3dib)
		IDirect3DIndexBuffer9_Release(impl->d3dib);

	crMem()->free(self, "CrBuffer");
}

CR_API CrBool crBufferInit(CrBuffer* self, CrBufferType type, size_t sizeInBytes, void* initialData)
{
	HRESULT hr;
	CrBufferImpl* impl = (CrBufferImpl*)self;
	self->sizeInBytes = sizeInBytes;
	self->type = type;

	if(self->type & CrBufferType_SysMem) {
		self->sysMem = crMem()->alloc(sizeInBytes, "CrBuffer");
	}
	else if(CrBufferType_Vertex == type) {
		hr = IDirect3DDevice9_CreateVertexBuffer(crContextImpl()->d3ddev, self->sizeInBytes, 0, 0, D3DPOOL_DEFAULT, &impl->d3dvb, nullptr);
		if(FAILED(hr)) {
			crDbgStr("d3d9 failed to create vertex buffer %8x", hr);
			return CrFalse;
		}
	}
	else if(CrBufferType_Index == type) {
		hr = IDirect3DDevice9_CreateIndexBuffer(crContextImpl()->d3ddev, self->sizeInBytes, 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &impl->d3dib, nullptr);
		if(FAILED(hr)) {
			crDbgStr("d3d9 failed to create index buffer %8x", hr);
			return CrFalse;
		}
	}
	else if(CrBufferType_Index32 == type) {
		hr = IDirect3DDevice9_CreateIndexBuffer(crContextImpl()->d3ddev, self->sizeInBytes, 0, D3DFMT_INDEX32, D3DPOOL_DEFAULT, &impl->d3dib, nullptr);
		if(FAILED(hr)) {
			crDbgStr("d3d9 failed to create index 32 buffer %8x", hr);
			return CrFalse;
		}
	}
	else {
		crDbgStr("uniform buffer is not supported on d3d9");
		return CrFalse;
	}

	if(nullptr != initialData) {
		crBufferUpdate(self, 0, sizeInBytes, initialData);
	}

	self->flags = CrBuffer_Inited;
	return CrTrue;
}

CR_API void crBufferUpdate(CrBuffer* self, size_t offsetInBytes, size_t sizeInBytes, void* data)
{
	HRESULT hr;
	CrBufferImpl* impl = (CrBufferImpl*)self;

	//UINT lockFlags = D3DLOCK_DISCARD;
	UINT lockFlags = 0;

	if(nullptr == self)
		return;

	if(offsetInBytes + sizeInBytes > self->sizeInBytes)
		return;

	if(self->type & CrBufferType_SysMem) {
		memcpy(((char*)self->sysMem) + offsetInBytes, data, sizeInBytes);
	}

	if(nullptr != impl->d3dvb) {
		void* ptr;
		hr = IDirect3DVertexBuffer9_Lock(impl->d3dvb, offsetInBytes, sizeInBytes, &ptr, lockFlags);
		if(FAILED(hr)) {
			return;
		}

		memcpy(ptr, data, sizeInBytes);

		IDirect3DVertexBuffer9_Unlock(impl->d3dvb);
	}

	if(nullptr != impl->d3dib) {
		void* ptr;
		hr = IDirect3DIndexBuffer9_Lock(impl->d3dib, offsetInBytes, sizeInBytes, &ptr, lockFlags);
		if(FAILED(hr)) {
			return;
		}

		memcpy(ptr, data, sizeInBytes);

		IDirect3DVertexBuffer9_Unlock(impl->d3dib);
	}
}

CR_API void* crBufferMap(CrBuffer* self, CrBufferMapAccess access)
{
	HRESULT hr;
	CrBufferImpl* impl = (CrBufferImpl*)self;
	void* ret = nullptr;

	//UINT lockFlags = D3DLOCK_DISCARD;
	UINT lockFlags = 0;
	
	if(nullptr == self)
		return nullptr;

	if(0 != (self->flags & CrBuffer_Mapped))
		return nullptr;

	if(self->type & CrBufferType_SysMem) {
		ret = self->sysMem;
	}

	if(nullptr != impl->d3dvb) {
		hr = IDirect3DVertexBuffer9_Lock(impl->d3dvb, 0, self->sizeInBytes, &ret, lockFlags);
		if(FAILED(hr)) {
			return nullptr;
		}
	}

	if(nullptr != impl->d3dib) {
		hr = IDirect3DIndexBuffer9_Lock(impl->d3dib, 0, self->sizeInBytes, &ret, lockFlags);
		if(FAILED(hr)) {
			return nullptr;
		}
	}

	self->flags |= CrBuffer_Mapped;

	return ret;
}

CR_API void crBufferUnmap(CrBuffer* self)
{
	CrBufferImpl* impl = (CrBufferImpl*)self;
	if(nullptr == self)
		return;

	if(0 == (self->flags & CrBuffer_Mapped))
		return;

	if(self->type & CrBufferType_SysMem) {
		// do nothing
	}

	if(nullptr != impl->d3dvb) {
		IDirect3DVertexBuffer9_Unlock(impl->d3dvb);
	}

	if(nullptr != impl->d3dib) {
		IDirect3DVertexBuffer9_Unlock(impl->d3dib);
	}

	self->flags &= ~CrBuffer_Mapped;
}
