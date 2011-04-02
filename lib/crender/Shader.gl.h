#ifndef __CRENDER_SHADER_GL_H__
#define __CRENDER_SHADER_GL_H__

#include "API.gl.h"
#include "Shader.h"
#include "uthash/uthash.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CrGpuShaderImpl
{
	CrGpuShader i;

	GLuint glName;

} CrGpuShaderImpl;

typedef struct CrGpuProgramUniform
{
	CrHashCode hash;
	GLuint loc;
	GLuint size;
	int texunit;
	UT_hash_handle hh;

} CrGpuProgramUniform;

typedef struct CrGpuProgramImpl
{
	CrGpuProgram i;

	GLuint glName;
	GLuint glVertexArray;
	CrGpuProgramUniform* cache;
	CrGpuProgramUniform* uniforms;

} CrGpuProgramImpl;

typedef struct CrInputGpuFormatMapping
{
	CrGpuFormat crFormat;
	int elemCnt;
	int elemType;
	int normalized;
	int stride;
} CrInputGpuFormatMapping;

#ifdef __cplusplus
}
#endif

#endif // __CRENDER_SHADER_GL_H__
