#ifndef __CRENDER_SHADER_D3D9_H__
#define __CRENDER_SHADER_D3D9_H__

#include "API.d3d9.h"
#include "Shader.h"
#include "uthash/uthash.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CrGpuShaderImpl
{
	CrGpuShader i;

	IDirect3DVertexShader9* d3dvs;
	IDirect3DPixelShader9* d3dps;
	//ID3DXBuffer* bytecode;
	ID3DXConstantTable* constTable;

} CrGpuShaderImpl;

typedef struct CrGpuProgramUniform
{
	CrHashCode hash;
	UINT loc;
	UINT size;
	int texunit;
	UT_hash_handle hh;

} CrGpuProgramUniform;

typedef struct CrGpuProgramInputAssembly
{
	size_t gpuInputId;
	IDirect3DVertexDeclaration9* d3ddecl;
	UT_hash_handle hh;

} CrGpuProgramInputAssembly;

typedef struct CrGpuProgramImpl
{
	CrGpuProgram i;

	IDirect3DVertexShader9* d3dvs;
	IDirect3DPixelShader9* d3dps;
	CrGpuProgramUniform* cacheVs;
	CrGpuProgramUniform* cachePs;
	CrGpuProgramUniform* uniformsVs;
	CrGpuProgramUniform* uniformsPs;
	CrGpuProgramInputAssembly* ias;

} CrGpuProgramImpl;

typedef struct CrInputGpuFormatMapping
{
	CrGpuFormat crFormat;
	D3DDECLTYPE declType;
	int stride;
} CrInputGpuFormatMapping;

#ifdef __cplusplus
}
#endif

#endif // __CRENDER_SHADER_D3D9_H__