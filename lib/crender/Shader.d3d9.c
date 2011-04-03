#include "Shader.d3d9.h"
#include "Texture.d3d9.h"
#include "Buffer.d3d9.h"
#include "Memory.h"
#include <stdio.h>

CR_API CrGpuShader* crGpuShaderAlloc()
{
	CrGpuShaderImpl* self = crMemory()->alloc(sizeof(CrGpuShaderImpl), "CrGpuShader");
	memset(self, 0, sizeof(CrGpuShaderImpl));
	return &self->i;
}

CR_API CrBool crGpuShaderInit(CrGpuShader* self, const char** sources, size_t srcCnt, CrGpuShaderType type)
{
	HRESULT hr;
	ID3DXBuffer* code;
	ID3DXBuffer* errors;
	ID3DXConstantTable* constTable;
	const char* profile = nullptr;
	CrGpuShaderImpl* impl = (CrGpuShaderImpl*)self;

	// compile shader
	if(CrGpuShaderType_Vertex == type) {
		profile = D3DXGetVertexShaderProfile(crAPI.d3ddev);
	}
	else if(CrGpuShaderType_Fragment == type) {
		profile = D3DXGetPixelShaderProfile(crAPI.d3ddev);
	}
	else {
		crDbgStr("d3d9 unsupported shader type %d", type);
		return CrFalse;
	}

	hr = D3DXCompileShader(sources[0], strlen(sources[0]),
		nullptr, nullptr, "main", profile,
		0, &code, &errors, &constTable
		);

	if(FAILED(hr)) {
		crDbgStr("d3d9 failed to compile shader\n%s", errors->lpVtbl->GetBufferPointer(errors));
		errors->lpVtbl->Release(errors);
		{
			FILE* fp = fopen("error-shader.txt", "w");
			fprintf(fp, "%s", sources[0]);
			fclose(fp);
		}
		return CrFalse;
	}

	// create shader
	if(CrGpuShaderType_Vertex == type) {
		hr = IDirect3DDevice9_CreateVertexShader(crAPI.d3ddev,
			code->lpVtbl->GetBufferPointer(code),
			&impl->d3dvs
			);
	}
	else if(CrGpuShaderType_Fragment == type) {
		hr = IDirect3DDevice9_CreatePixelShader(crAPI.d3ddev,
			code->lpVtbl->GetBufferPointer(code),
			&impl->d3dps
			);
	}

	if(FAILED(hr)) {
		crDbgStr("d3d9 failed to create shader %8x", hr);
		
		constTable->lpVtbl->Release(constTable);
		return CrFalse;
	}
	
	code->lpVtbl->Release(code);

	// save the byte code for later use
	//impl->bytecode = code;
	impl->constTable = constTable;

	self->type = type;
	self->flags |= CrGpuShader_Inited;

	return CrTrue;
}

CR_API void crGpuShaderFree(CrGpuShader* self)
{
	CrGpuShaderImpl* impl = (CrGpuShaderImpl*)self;

	if(nullptr == self)
		return;

	if(nullptr != impl->d3dvs) {
		IDirect3DVertexShader9_Release(impl->d3dvs);
	}
	if(nullptr != impl->d3dps) {
		IDirect3DPixelShader9_Release(impl->d3dps);
	}
	//if(nullptr != impl->bytecode) {
	//	impl->bytecode->lpVtbl->Release(impl->bytecode);
	//}
	if(nullptr != impl->constTable) {
		impl->constTable->lpVtbl->Release(impl->constTable);
	}

	crMemory()->free(self, "crGpuShader");
}

CR_API CrGpuProgram* crGpuProgramAlloc()
{
	CrGpuProgramImpl* self = crMemory()->alloc(sizeof(CrGpuProgramImpl), "CrGpuProgram");
	memset(self, 0, sizeof(CrGpuProgramImpl));
	return &self->i;
}

CR_API void crGpuProgramUniformCollect(CrGpuShader* self, CrGpuProgramUniform** table, CrGpuProgramUniform** uniforms)
{
	UINT i;
	ID3DXConstantTable* constTable = ((CrGpuShaderImpl*)self)->constTable;
	
	{
		D3DXCONSTANTTABLE_DESC tblDesc;
		constTable->lpVtbl->GetDesc(constTable, &tblDesc);

		*uniforms = crMemory()->alloc(sizeof(CrGpuProgramUniform) * tblDesc.Constants, "CrGpuProgram");
		memset(*uniforms, 0, sizeof(CrGpuProgramUniform) * tblDesc.Constants);

		for(i=0; i<tblDesc.Constants; ++i) {
			CrGpuProgramUniform* uniform;
			D3DXHANDLE h = constTable->lpVtbl->GetConstant(constTable, nullptr, i);
			D3DXCONSTANT_DESC constDesc;
			UINT cnt = 1;
			if(D3D_OK != constTable->lpVtbl->GetConstantDesc(constTable, h, &constDesc, &cnt)) {
				continue;
			}

			uniform = &((*uniforms)[i]);
			uniform->loc = constDesc.RegisterIndex;
			uniform->size = constDesc.RegisterCount;
			uniform->hash = CrHash(constDesc.Name);

			if(D3DXRS_SAMPLER == constDesc.RegisterSet) {
				uniform->texunit = constTable->lpVtbl->GetSamplerIndex(constTable, h);
			}
			else {
				uniform->texunit = -1;
			}
			HASH_ADD_INT(*table, hash, uniform);
		}
	}
}

CR_API CrBool crGpuProgramInit(CrGpuProgram* self, CrGpuShader** shaders, size_t shaderCnt)
{
	CrGpuProgramImpl* impl = (CrGpuProgramImpl*)self;
	CrGpuShader* vs = nullptr;
	CrGpuShader* ps = nullptr;

	if(self->flags & CrGpuProgram_Inited) {
		crDbgStr("CrGpuProgram already inited!\n");
		return CrFalse;
	}

#define COM_ADD_REF(x) x->lpVtbl->AddRef(x)

	// attach shaders
	{
		size_t i;
		for(i=0; i<shaderCnt; ++i) {
			if(nullptr == shaders[i])
				continue;
			
			switch(shaders[i]->type) {
				case CrGpuShaderType_Vertex:
					{
						vs = shaders[i];
						impl->d3dvs = ((CrGpuShaderImpl*)shaders[i])->d3dvs;
						COM_ADD_REF(impl->d3dvs);
						break;
					}
				case CrGpuShaderType_Fragment:
					{
						ps = shaders[i];
						impl->d3dps = ((CrGpuShaderImpl*)shaders[i])->d3dps;
						COM_ADD_REF(impl->d3dps);
						break;
					}
			}
		}
	}
#undef COM_ADD_REF

	if(nullptr == impl->d3dvs || nullptr == impl->d3dps) {
		crDbgStr("d3d9 incomplete program vs:%d ps:%d", impl->d3dvs, impl->d3dps);
		return CrFalse;
	}

	// collect uniforms
	crGpuProgramUniformCollect(vs, &impl->cacheVs, &impl->uniformsVs);
	crGpuProgramUniformCollect(ps, &impl->cachePs, &impl->uniformsPs);

	self->flags |= CrGpuProgram_Inited;

	return CrTrue;
	
}

CR_API void crGpuProgramFree(CrGpuProgram* self)
{
	CrGpuProgramImpl* impl = (CrGpuProgramImpl*)self;

	if(nullptr == self)
		return;

	crMemory()->free(impl->uniformsVs, "crGpuProgram");
	crMemory()->free(impl->uniformsPs, "crGpuProgram");
	
	if(nullptr != impl->d3dvs) {
		IDirect3DVertexShader9_Release(impl->d3dvs);
	}

	if(nullptr != impl->d3dps) {
		IDirect3DPixelShader9_Release(impl->d3dps);
	}

	{
		CrGpuProgramInputAssembly* curr, *temp;
		HASH_ITER(hh, impl->ias, curr, temp) {
			HASH_DEL(impl->ias, curr);
			IDirect3DVertexDeclaration9_Release(curr->d3ddecl);
			crMemory()->free(curr, "crGpuProgram");
		}
	}

	crMemory()->free(self, "crGpuProgram");
}

CR_API void crGpuProgramPreRender(CrGpuProgram* self)
{
	CrGpuProgramImpl* impl = (CrGpuProgramImpl*)self;

	if(nullptr == self)
		return;

	if(0 == (self->flags & CrGpuProgram_Inited)) {
		//crDbgStr("CrGpuProgram is not inited!\n");
		return;
	}

	IDirect3DDevice9_SetVertexShader(crAPI.d3ddev, impl->d3dvs);
	IDirect3DDevice9_SetPixelShader(crAPI.d3ddev, impl->d3dps);
}

CR_API CrBool crGpuProgramUniformfv(CrGpuProgram* self, CrHashCode hash, size_t count, const float* value)
{
	CrGpuProgramUniform* uniform;
	CrGpuProgramImpl* impl = (CrGpuProgramImpl*)self;

	if(nullptr == self)
		return CrFalse;

	if(0 == (self->flags & CrGpuProgram_Inited)) {
		//crDbgStr("CrGpuProgram is not inited!\n");
		return CrFalse;
	}
	
	HASH_FIND_INT(impl->cacheVs, &hash, uniform);
	if(nullptr != uniform) {
		IDirect3DDevice9_SetVertexShaderConstantF(crAPI.d3ddev, uniform->loc, value, uniform->size * count);
	}

	HASH_FIND_INT(impl->cachePs, &hash, uniform);
	if(nullptr != uniform) {
		IDirect3DDevice9_SetPixelShaderConstantF(crAPI.d3ddev, uniform->loc, value, uniform->size * count);
	}
	
	return CrTrue;
}

CR_API CrBool crGpuProgramUniform1fv(CrGpuProgram* self, CrHashCode hash, size_t count, const float* value)
{
	return crGpuProgramUniformfv(self, hash, count, value);
}

CR_API CrBool crGpuProgramUniform2fv(CrGpuProgram* self, CrHashCode hash, size_t count, const float* value)
{
	return crGpuProgramUniformfv(self, hash, count, value);
}

CR_API CrBool crGpuProgramUniform3fv(CrGpuProgram* self, CrHashCode hash, size_t count, const float* value)
{
	return crGpuProgramUniformfv(self, hash, count, value);
}

CR_API CrBool crGpuProgramUniform4fv(CrGpuProgram* self, CrHashCode hash, size_t count, const float* value)
{
	return crGpuProgramUniformfv(self, hash, count, value);
}

CR_API CrBool crGpuProgramUniformMtx4fv(CrGpuProgram* self, CrHashCode hash, size_t count, CrBool transpose, const float* value)
{
	return crGpuProgramUniformfv(self, hash, count, value);
}

static D3DTEXTUREFILTERTYPE crD3D9_SAMPLER_MAG_FILTER[] = {
	D3DTEXF_POINT,
	D3DTEXF_LINEAR,
	D3DTEXF_POINT,
	D3DTEXF_LINEAR,
	D3DTEXF_POINT,
	D3DTEXF_LINEAR,
};

static D3DTEXTUREFILTERTYPE crD3D9_SAMPLER_MIN_FILTER[] = {
	D3DTEXF_POINT,
	D3DTEXF_LINEAR,
	D3DTEXF_POINT,
	D3DTEXF_LINEAR,
	D3DTEXF_POINT,
	D3DTEXF_LINEAR,
};

static D3DTEXTUREFILTERTYPE crD3D9_SAMPLER_MIP_FILTER[] = {
	D3DTEXF_POINT,
	D3DTEXF_LINEAR,
	D3DTEXF_LINEAR,
	D3DTEXF_POINT,
	D3DTEXF_NONE,
	D3DTEXF_NONE,
};

static D3DTEXTUREADDRESS crD3D9_SAMPLER_ADDRESS[] = {
	D3DTADDRESS_WRAP,
	D3DTADDRESS_CLAMP,
};

CR_API CrBool crGpuProgramUniformTexture(CrGpuProgram* self, CrHashCode hash, struct CrTexture* texture, const struct CrSampler* sampler)
{
	CrGpuProgramUniform* uniform;
	CrGpuProgramImpl* impl = (CrGpuProgramImpl*)self;

	if(nullptr == self)
		return CrFalse;

	if(0 == (self->flags & CrGpuProgram_Inited)) {
		//crDbgStr("CrGpuProgram is not inited!\n");
		return CrFalse;
	}
	
	HASH_FIND_INT(impl->cacheVs, &hash, uniform);
	if(nullptr != uniform && -1 != uniform->texunit) {
		IDirect3DDevice9_SetTexture(crAPI.d3ddev, uniform->texunit, (IDirect3DBaseTexture9*)((CrTextureImpl*)texture)->d3dtex);
	}

	HASH_FIND_INT(impl->cachePs, &hash, uniform);
	if(nullptr != uniform) {
		IDirect3DDevice9_SetTexture(crAPI.d3ddev, uniform->texunit, (IDirect3DBaseTexture9*)((CrTextureImpl*)texture)->d3dtex);
		IDirect3DDevice9_SetSamplerState(crAPI.d3ddev, uniform->texunit, D3DSAMP_MAGFILTER, crD3D9_SAMPLER_MAG_FILTER[sampler->filter]);
		IDirect3DDevice9_SetSamplerState(crAPI.d3ddev, uniform->texunit, D3DSAMP_MINFILTER, crD3D9_SAMPLER_MIN_FILTER[sampler->filter]);
		IDirect3DDevice9_SetSamplerState(crAPI.d3ddev, uniform->texunit, D3DSAMP_MIPFILTER, crD3D9_SAMPLER_MIP_FILTER[sampler->filter]);
		IDirect3DDevice9_SetSamplerState(crAPI.d3ddev, uniform->texunit, D3DSAMP_ADDRESSU, crD3D9_SAMPLER_ADDRESS[sampler->addressU]);
		IDirect3DDevice9_SetSamplerState(crAPI.d3ddev, uniform->texunit, D3DSAMP_ADDRESSV, crD3D9_SAMPLER_ADDRESS[sampler->addressV]);
		IDirect3DDevice9_SetSamplerState(crAPI.d3ddev, uniform->texunit, D3DSAMP_ADDRESSW, crD3D9_SAMPLER_ADDRESS[sampler->addressW]);
	}
	
	return CrTrue;
}


CrInputGpuFormatMapping CrInputGpuFormatMappings[] = {
	{CrGpuFormat_FloatR32,				D3DDECLTYPE_FLOAT1, sizeof(float)},
	{CrGpuFormat_FloatR32G32,			D3DDECLTYPE_FLOAT2, sizeof(float) * 2},
	{CrGpuFormat_FloatR32G32B32,		D3DDECLTYPE_FLOAT3, sizeof(float) * 3},
	{CrGpuFormat_FloatR32G32B32A32,	D3DDECLTYPE_FLOAT4, sizeof(float) * 4},
};

CrInputGpuFormatMapping* crInputGpuFormatMappingGet(CrGpuFormat crFormat)
{
	size_t i=0;
	for(i=0; i<crCountOf(CrInputGpuFormatMappings); ++i) {
		CrInputGpuFormatMapping* mapping = &CrInputGpuFormatMappings[i];
		if(crFormat == mapping->crFormat)
			return mapping;
	}

	return nullptr;
}

D3DDECLUSAGE crGpuInputGetUsage(CrGpuProgramInput* input)
{
	if(strstr(input->name, "pos") || strstr(input->name, "vertex"))
		return D3DDECLUSAGE_POSITION;

	if(strstr(input->name, "nor"))
		return D3DDECLUSAGE_NORMAL;

	if(strstr(input->name, "col"))
		return D3DDECLUSAGE_COLOR;
	
	if(strstr(input->name, "tex"))
		return D3DDECLUSAGE_TEXCOORD;

	return D3DDECLUSAGE_POSITION;
}

BYTE crGpuInputGetUsageIndex(CrGpuProgramInput* input)
{
	size_t len = strlen(input->name);
	char ch;

	if(0 == len)
		return 0;

	ch = input->name[len-1];

	if(ch < '0' || ch > '9')
		return 0;

	return ch - '0';
}

void crGpuProgramBindBuffer(CrGpuProgram* self, CrGpuProgramInput* inputs, size_t count)
{
	size_t i = 0;
	size_t stream = 0;
	CrBufferImpl* lastBuffer = nullptr;

	for(i=0; i<count; ++i) {
		CrGpuProgramInput* input = &inputs[i];
		CrBufferImpl* buffer = (CrBufferImpl*)input->buffer;
		CrInputGpuFormatMapping* m = crInputGpuFormatMappingGet(input->format);

		if(nullptr == buffer)
			continue;

		if(CrBufferType_Index == buffer->i.type) {
			// bind index buffer
			IDirect3DDevice9_SetIndices(crAPI.d3ddev, buffer->d3dib);
		}
		else if(CrBufferType_Vertex == buffer->i.type) {
			// bind vertex buffer
			if(lastBuffer == buffer) 
				continue;

			if(nullptr != lastBuffer)
				++stream;

			IDirect3DDevice9_SetStreamSource(crAPI.d3ddev, stream, buffer->d3dvb, 0, m->stride);
			lastBuffer = buffer;
		}
	}
}

static D3DVERTEXELEMENT9 crD3D9_ELEM_END = D3DDECL_END();
static D3DVERTEXELEMENT9 crD3D9_ELEMS[16];

void crGpuProgramBindVertexDecl(CrGpuProgram* self, size_t gpuInputId, CrGpuProgramInput* inputs, size_t count)
{
	CrGpuProgramInputAssembly* ia;
	CrGpuProgramImpl* impl = (CrGpuProgramImpl*)self;

	HASH_FIND_INT(impl->ias, &gpuInputId, ia);
	if(nullptr == ia) {

		size_t i = 0;
		size_t stream = 0;
		size_t offset = 0;
		size_t elem = 0;
		CrBufferImpl* lastBuffer = nullptr;

		for(i=0; i<count; ++i) {
			CrGpuProgramInput* input = &inputs[i];
			CrBufferImpl* buffer = (CrBufferImpl*)input->buffer;
			CrInputGpuFormatMapping* m = crInputGpuFormatMappingGet(input->format);

			if(nullptr == buffer)
				continue;

			if(CrBufferType_Vertex != buffer->i.type)
				continue;

			if(lastBuffer != buffer) {
				if(nullptr != lastBuffer) {
					++stream;
					offset = 0;
				}
				lastBuffer = buffer;
			}

			crD3D9_ELEMS[elem].Stream = stream;
			crD3D9_ELEMS[elem].Offset = offset;
			crD3D9_ELEMS[elem].Type = m->declType;
			crD3D9_ELEMS[elem].Method = D3DDECLMETHOD_DEFAULT;
			crD3D9_ELEMS[elem].Usage = crGpuInputGetUsage(input);
			crD3D9_ELEMS[elem].UsageIndex = crGpuInputGetUsageIndex(input);

			++elem;
			offset += m->stride;
		}
		crD3D9_ELEMS[elem] = crD3D9_ELEM_END;

		ia = crMemory()->alloc(sizeof(CrGpuProgramInputAssembly), "CrGpuProgram");
		ia->gpuInputId = gpuInputId;
		IDirect3DDevice9_CreateVertexDeclaration(crAPI.d3ddev, crD3D9_ELEMS, &ia->d3ddecl);

		HASH_ADD_INT(impl->ias, gpuInputId, ia);
	}

	IDirect3DDevice9_SetVertexDeclaration(crAPI.d3ddev, ia->d3ddecl);
}

CR_API size_t crGenGpuInputId()
{
	return ++crAPI.gpuInputId;
}

CR_API void crGpuProgramBindInput(CrGpuProgram* self, size_t gpuInputId, CrGpuProgramInput* inputs, size_t count)
{
	crGpuProgramBindBuffer(self, inputs, count);
	crGpuProgramBindVertexDecl(self, gpuInputId, inputs, count);
}

CR_API void crGpuDrawPoint(size_t offset, size_t count)
{
	IDirect3DDevice9_DrawPrimitive(crAPI.d3ddev, D3DPT_POINTLIST, offset, count);
}

CR_API void crGpuDrawLine(size_t offset, size_t count, size_t flags)
{
	D3DPRIMITIVETYPE mode = (flags & CrGpuDraw_Stripped) ? D3DPT_LINESTRIP : D3DPT_LINELIST;
	IDirect3DDevice9_DrawPrimitive(crAPI.d3ddev, mode, offset, count / 2);
}

CR_API void crGpuDrawLineIndexed(size_t offset, size_t count, size_t minIdx, size_t maxIdx, size_t flags)
{
	D3DPRIMITIVETYPE mode = (flags & CrGpuDraw_Stripped) ? D3DPT_LINESTRIP : D3DPT_LINELIST;
	IDirect3DDevice9_DrawIndexedPrimitive(crAPI.d3ddev, mode, 0, minIdx, maxIdx+1, offset, count / 2);
}

CR_API void crGpuDrawTriangle(size_t offset, size_t count, size_t flags)
{
	D3DPRIMITIVETYPE mode = (flags & CrGpuDraw_Stripped) ? D3DPT_TRIANGLESTRIP : D3DPT_TRIANGLELIST;
	IDirect3DDevice9_DrawPrimitive(crAPI.d3ddev, mode, offset, count / 3);
}

CR_API void crGpuDrawTriangleIndexed(size_t offset, size_t count, size_t minIdx, size_t maxIdx, size_t flags)
{
	D3DPRIMITIVETYPE mode = (flags & CrGpuDraw_Stripped) ? D3DPT_TRIANGLESTRIP : D3DPT_TRIANGLELIST;
	IDirect3DDevice9_DrawIndexedPrimitive(crAPI.d3ddev, mode, 0, minIdx, maxIdx+1, offset, count / 3);
}

CR_API void crGpuDrawPatch(size_t offset, size_t count, size_t vertexPerPatch, size_t flags)
{
	// not supported
}

CR_API void crGpuDrawPatchIndexed(size_t offset, size_t count, size_t minIdx, size_t maxIdx, size_t vertexPerPatch, size_t flags)
{
	// not supported
}
