#ifndef __CRENDER_SHADER_GL_H__
#define __CRENDER_SHADER_GL_H__

#include "Context.gl.h"
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

typedef struct CrGpuProgramAttribute
{
	CrHashCode hash;
	GLuint loc;
	UT_hash_handle hh;

} CrGpuProgramAttribute;

typedef struct CrGpuProgramImpl
{
	CrGpuProgram i;

	GLuint glName;
	//GLuint glVertexArray;
	CrGpuProgramUniform* uniformCache;
	CrGpuProgramUniform* uniforms;
	CrGpuProgramAttribute* attrCache;
	CrGpuProgramAttribute* attrs;

} CrGpuProgramImpl;

typedef struct CrInputGpuFormatMapping
{
	CrGpuFormat crFormat;
	int elemCnt;
	int elemType;
	GLboolean normalized;
	int stride;
} CrInputGpuFormatMapping;

#ifdef __cplusplus
}
#endif

#endif // __CRENDER_SHADER_GL_H__
