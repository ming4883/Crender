#ifndef __CRENDER_SHADER_H__
#define __CRENDER_SHADER_H__

#include "Platform.h"
#include "StrHash.h"
#include "GpuFormat.h"

#ifdef __cplusplus
extern "C" {
#endif

struct CrTexture;
struct CrBuffer;

typedef enum CrSamplerFilter
{
	CrSamplerFilter_MagMinMip_Nearest,
	CrSamplerFilter_MagMinMip_Linear,
	CrSamplerFilter_MagMin_Nearest_Mip_Linear,
	CrSamplerFilter_MagMin_Linear_Mip_Nearest,
	CrSamplerFilter_MagMin_Nearest_Mip_None,
	CrSamplerFilter_MagMin_Linear_Mip_None,

} CrSamplerFilter;

typedef enum CrSamplerAddress
{
	CrSamplerAddress_Wrap,
	CrSamplerAddress_Clamp,

} CrSamplerAddress;

typedef struct CrSampler
{
	CrSamplerFilter filter;
	CrSamplerAddress addressU, addressV, addressW;

} CrSampler;


// CrGpuShader
typedef enum CrGpuShaderType
{
	CrGpuShaderType_Vertex,
	CrGpuShaderType_Fragment,
	CrGpuShaderType_Geometry,
	CrGpuShaderType_TessControl,
	CrGpuShaderType_TessEvaluation,
} CrGpuShaderType;

typedef enum CrGpuShaderFlag
{
	CrGpuShader_Inited = 1 << 0,
} CrGpuShaderFlag;

typedef struct CrGpuShader
{
	size_t flags;
	CrGpuShaderType type;

} CrGpuShader;


CrGpuShader* crGpuShaderAlloc();

void crGpuShaderFree(CrGpuShader* self);

CrBool crGpuShaderInit(CrGpuShader* self, const char** sources, size_t srcCnt, CrGpuShaderType type);

// CrGpuProgram
typedef enum CrGpuProgramFlag
{
	CrGpuProgram_Inited = 1 << 0,
} CrGpuProgramFlag;

typedef struct CrGpuProgram
{
	size_t flags;

} CrGpuProgram;

CrGpuProgram* crGpuProgramAlloc();

void crGpuProgramFree(CrGpuProgram* self);

CrBool crGpuProgramInit(CrGpuProgram* self, CrGpuShader** shaders, size_t shaderCnt);

void crGpuProgramPreRender(CrGpuProgram* self);

CrBool crGpuProgramUniform1fv(CrGpuProgram* self, CrHashCode hash, size_t count, const float* value);
CrBool crGpuProgramUniform2fv(CrGpuProgram* self, CrHashCode hash, size_t count, const float* value);
CrBool crGpuProgramUniform3fv(CrGpuProgram* self, CrHashCode hash, size_t count, const float* value);
CrBool crGpuProgramUniform4fv(CrGpuProgram* self, CrHashCode hash, size_t count, const float* value);
CrBool crGpuProgramUniformMtx4fv(CrGpuProgram* self, CrHashCode hash, size_t count, CrBool transpose, const float* value);
CrBool crGpuProgramUniformTexture(CrGpuProgram* self, CrHashCode hash, struct CrTexture* texture, const struct CrSampler* sampler);

typedef struct CrGpuProgramInput
{
	struct CrBuffer* buffer;
	const char* name;
	size_t offset;
	CrGpuFormat format;
} CrGpuProgramInput;

size_t crGenGpuInputId();

void crGpuProgramBindInput(CrGpuProgram* self, size_t gpuInputId, CrGpuProgramInput* inputs, size_t count);

typedef enum CrGpuDrawFlag
{
	CrGpuDraw_Stripped		= 0x0001,	//!< draw primitive strips, not supported with point and patch.

	CrGpuDraw_Indexed		= 0x0010,	//!< draw with a 16-bit index buffer
	CrGpuDraw_Indexed8		= 0x0011,	//!< draw with a 8-bit index buffer
	CrGpuDraw_Indexed32	= 0x0012,	//!< draw with a 32-bit index buffer
	
} CrGpuDrawFlag;

void crGpuDrawPoint(size_t offset, size_t count);

void crGpuDrawLine(size_t offset, size_t count, size_t flags);
void crGpuDrawLineIndexed(size_t offset, size_t count, size_t minIdx, size_t maxIdx, size_t flags);

void crGpuDrawTriangle(size_t offset, size_t count, size_t flags);
void crGpuDrawTriangleIndexed(size_t offset, size_t count, size_t minIdx, size_t maxIdx, size_t flags);

void crGpuDrawPatch(size_t offset, size_t count, size_t vertexPerPatch, size_t flags);
void crGpuDrawPatchIndexed(size_t offset, size_t count, size_t minIdx, size_t maxIdx, size_t vertexPerPatch, size_t flags);

#ifdef __cplusplus
}
#endif

#endif // __CRENDER_SHADER_H__