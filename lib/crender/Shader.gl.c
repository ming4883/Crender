#include "Shader.gl.h"
#include "Texture.gl.h"
#include "Buffer.gl.h"
#include "Mem.h"
#include <stdio.h>

static GLenum crGL_SHADER_TYPE[] = {
	GL_VERTEX_SHADER,
	GL_FRAGMENT_SHADER,
#if !defined(CR_GLES_2)
	GL_GEOMETRY_SHADER,
	GL_TESS_CONTROL_SHADER,
	GL_TESS_EVALUATION_SHADER,
#endif
};

CR_API CrGpuShader* crGpuShaderAlloc()
{
	CrGpuShaderImpl* self = crMem()->alloc(sizeof(CrGpuShaderImpl), "CrGpuShader");
	memset(self, 0, sizeof(CrGpuShaderImpl));
	return &self->i;
}

CR_API CrBool crGpuShaderInit(CrGpuShader* self, const char** sources, size_t srcCnt, CrGpuShaderType type)
{
	int compileStatus;
	CrGpuShaderImpl* impl = (CrGpuShaderImpl*)self;

	if(self->flags & CrGpuShader_Inited) {
		crDbgStr("CrGpuShader already inited!\n");
		return CrFalse;
	}

	self->flags = 0;
	self->type = type;
	impl->glName = glCreateShader(crGL_SHADER_TYPE[self->type]);

	glShaderSource(impl->glName, srcCnt, sources, nullptr);
	glCompileShader(impl->glName);

	glGetShaderiv(impl->glName, GL_COMPILE_STATUS, &compileStatus);

	if(GL_FALSE == compileStatus) {
		GLint len;
		glGetShaderiv(impl->glName, GL_INFO_LOG_LENGTH, &len);
		if(len > 0) {
			char* buf = (char*)crMem()->alloc(len, "CrGpuShader");
			glGetShaderInfoLog(impl->glName, len, nullptr, buf);
			crDbgStr("glCompileShader failed: %s", buf);
			crMem()->free(buf, "CrGpuShader");
		}

		return CrFalse;
	}
	else {
		self->flags |= CrGpuShader_Inited;
	}

	return CrTrue;
}

CR_API void crGpuShaderFree(CrGpuShader* self)
{
	CrGpuShaderImpl* impl = (CrGpuShaderImpl*)self;

	if(nullptr == self)
		return;

	glDeleteShader(impl->glName);
	crMem()->free(self, "CrGpuShader");
}

CR_API CrGpuProgram* crGpuProgramAlloc()
{
	CrGpuProgramImpl* self = crMem()->alloc(sizeof(CrGpuProgramImpl), "CrGpuProgram");
	memset(self, 0, sizeof(CrGpuProgramImpl));
	return &self->i;
}

CR_API CrBool crGpuProgramInit(CrGpuProgram* self, CrGpuShader** shaders, size_t shaderCnt)
{
	size_t i;
	int linkStatus;
	CrGpuProgramImpl* impl = (CrGpuProgramImpl*)self;

	if(self->flags & CrGpuProgram_Inited) {
		crDbgStr("CrGpuProgram already inited!\n");
		return CrFalse;
	}

	impl->glName = glCreateProgram();

	// attach shaders
	for(i=0; i<shaderCnt; ++i) {
		if(nullptr != shaders[i]) {
			glAttachShader(impl->glName, ((CrGpuShaderImpl*)shaders[i])->glName);
		}
	}

	// link program
	glLinkProgram(impl->glName);

	glGetProgramiv(impl->glName, GL_LINK_STATUS, &linkStatus);
	if(GL_FALSE == linkStatus) {
		GLint len;
		glGetProgramiv(impl->glName, GL_INFO_LOG_LENGTH, &len);
		if(len > 0) {
			char* buf = (char*)crMem()->alloc(len, "CrGpuProgram");
			glGetProgramInfoLog(impl->glName, len, nullptr, buf);
			crDbgStr("glLinkProgram failed: %s", buf);
			crMem()->free(buf, "CrGpuProgram");
		}
		return CrFalse;
	}

	self->flags |= CrGpuProgram_Inited;

	glUseProgram(impl->glName);

	// query all uniforms
	{
		GLint i;
		GLint uniformCnt;
		GLsizei uniformLength;
		GLint uniformSize;
		GLenum uniformType;
		char uniformName[32];
		GLuint texunit = 0;

		glGetProgramiv(impl->glName, GL_ACTIVE_UNIFORMS, &uniformCnt);

		impl->uniforms = crMem()->alloc(sizeof(CrGpuProgramUniform) * uniformCnt, "CrGpuProgram");
		memset(impl->uniforms, 0, sizeof(CrGpuProgramUniform) * uniformCnt);
		crDbgStr("glProgram %d has %d uniforms\n", impl->glName, uniformCnt);

		for(i=0; i<uniformCnt; ++i) {
			CrGpuProgramUniform* uniform;
			glGetActiveUniform(impl->glName, i, crCountOf(uniformName), &uniformLength, &uniformSize, &uniformType, uniformName);
			uniform = &impl->uniforms[i];
			uniform->hash = CrHash(uniformName);
			uniform->loc = glGetUniformLocation(impl->glName, uniformName);
			uniform->size = uniformSize;
			uniform->texunit = texunit;

			HASH_ADD_INT(impl->cache, hash, uniform);

			switch(uniformType) {
				case GL_SAMPLER_2D:
				case GL_SAMPLER_CUBE:
#if !defined(CR_GLES_2)
				case GL_SAMPLER_1D:
				case GL_SAMPLER_3D:
				case GL_SAMPLER_1D_SHADOW:
				case GL_SAMPLER_2D_SHADOW:
#endif
					{	// bind sampler to the specific texture unit
						glUniform1i(i, texunit++);
					}
					break;
				default:
					uniform->texunit = -1;
					break;
			}
			//crDbgStr("%s %d %d 0x%04x %d\n", uniformName, i, uniformSize, uniformType, uniform->texunit);
		}

	}

#if !defined(CR_GLES_2)
	glGenVertexArrays(1, &impl->glVertexArray);
#endif

	return CrTrue;

}

CR_API void crGpuProgramFree(CrGpuProgram* self)
{
	CrGpuProgramImpl* impl = (CrGpuProgramImpl*)self;
	if(nullptr == self)
		return;

	HASH_CLEAR(hh, impl->cache);
	crMem()->free(impl->uniforms, "CrGpuProgram");

	glDeleteProgram(impl->glName);

#if !defined(CR_GLES_2)
	glDeleteVertexArrays(1, &impl->glVertexArray);
#endif

	crMem()->free(self, "CrGpuProgram");
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

	glUseProgram(impl->glName);
}

CR_API CrBool crGpuProgramUniform1fv(CrGpuProgram* self, CrHashCode hash, size_t count, const float* value)
{
	CrGpuProgramUniform* uniform;
	CrGpuProgramImpl* impl = (CrGpuProgramImpl*)self;

	if(nullptr == self)
		return CrFalse;

	if(0 == (self->flags & CrGpuProgram_Inited)) {
		//crDbgStr("CrGpuProgram is not inited!\n");
		return CrFalse;
	}

	HASH_FIND_INT(impl->cache, &hash, uniform);
	if(nullptr == uniform)
		return CrFalse;

	glUniform1fv(uniform->loc, count, value);
	return CrTrue;
}

CR_API CrBool crGpuProgramUniform2fv(CrGpuProgram* self, CrHashCode hash, size_t count, const float* value)
{
	CrGpuProgramUniform* uniform;
	CrGpuProgramImpl* impl = (CrGpuProgramImpl*)self;

	if(nullptr == self)
		return CrFalse;

	if(0 == (self->flags & CrGpuProgram_Inited)) {
		//crDbgStr("CrGpuProgram is not inited!\n");
		return CrFalse;
	}

	HASH_FIND_INT(impl->cache, &hash, uniform);
	if(nullptr == uniform)
		return CrFalse;

	glUniform2fv(uniform->loc, count, value);
	return CrTrue;
}

CR_API CrBool crGpuProgramUniform3fv(CrGpuProgram* self, CrHashCode hash, size_t count, const float* value)
{
	CrGpuProgramUniform* uniform;
	CrGpuProgramImpl* impl = (CrGpuProgramImpl*)self;

	if(nullptr == self)
		return CrFalse;

	if(0 == (self->flags & CrGpuProgram_Inited)) {
		//crDbgStr("CrGpuProgram is not inited!\n");
		return CrFalse;
	}

	HASH_FIND_INT(impl->cache, &hash, uniform);
	if(nullptr == uniform)
		return CrFalse;

	glUniform3fv(uniform->loc, count, value);
	return CrTrue;
}

CR_API CrBool crGpuProgramUniform4fv(CrGpuProgram* self, CrHashCode hash, size_t count, const float* value)
{
	CrGpuProgramUniform* uniform;
	CrGpuProgramImpl* impl = (CrGpuProgramImpl*)self;

	if(nullptr == self)
		return CrFalse;

	if(0 == (self->flags & CrGpuProgram_Inited)) {
		//crDbgStr("CrGpuProgram is not inited!\n");
		return CrFalse;
	}

	HASH_FIND_INT(impl->cache, &hash, uniform);
	if(nullptr == uniform)
		return CrFalse;

	glUniform4fv(uniform->loc, count, value);
	return CrTrue;
}

CR_API CrBool crGpuProgramUniformMtx4fv(CrGpuProgram* self, CrHashCode hash, size_t count, CrBool transpose, const float* value)
{
	CrGpuProgramUniform* uniform;
	CrGpuProgramImpl* impl = (CrGpuProgramImpl*)self;

	if(nullptr == self)
		return CrFalse;

	if(0 == (self->flags & CrGpuProgram_Inited)) {
		//crDbgStr("CrGpuProgram is not inited!\n");
		return CrFalse;
	}

	HASH_FIND_INT(impl->cache, &hash, uniform);
	if(nullptr == uniform)
		return CrFalse;

	glUniformMatrix4fv(uniform->loc, count, transpose, value);
	//crDbgStr("uniform %d %f %f %f %f\n", uniform->loc, value[0], value[1], value[2], value[3]);
	return CrTrue;
}

static GLenum crGL_SAMPLER_MIN_FILTER[] = {
	GL_NEAREST_MIPMAP_NEAREST,
	GL_LINEAR_MIPMAP_LINEAR,
	GL_NEAREST_MIPMAP_LINEAR,
	GL_LINEAR_MIPMAP_NEAREST,
	GL_NEAREST,
	GL_LINEAR,
};

static GLenum crGL_SAMPLER_MAG_FILTER[] = {
	GL_NEAREST,
	GL_LINEAR,
	GL_NEAREST,
	GL_LINEAR,
	GL_NEAREST,
	GL_LINEAR,
};

static GLenum crGL_SAMPLER_ADDRESS[] = {
	GL_REPEAT,
	GL_CLAMP_TO_EDGE,
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

	HASH_FIND_INT(impl->cache, &hash, uniform);
	if(nullptr == uniform)
		return CrFalse;

	if(uniform->texunit < 0) {
		crDbgStr("Not a texture!\n");
		return CrFalse;
	}

	glActiveTexture(GL_TEXTURE0 + uniform->texunit);
	if(nullptr == texture)
		glBindTexture(GL_TEXTURE_2D, 0);
	else {
		int gltarget = ((CrTextureImpl*)texture)->glTarget;
		int glname = ((CrTextureImpl*)texture)->glName;
		glBindTexture(gltarget, glname);
		glTexParameteri(gltarget, GL_TEXTURE_MAG_FILTER, crGL_SAMPLER_MAG_FILTER[sampler->filter]);
		glTexParameteri(gltarget, GL_TEXTURE_MIN_FILTER, crGL_SAMPLER_MIN_FILTER[sampler->filter]);
		glTexParameteri(gltarget, GL_TEXTURE_WRAP_S, crGL_SAMPLER_ADDRESS[sampler->addressU]);
		glTexParameteri(gltarget, GL_TEXTURE_WRAP_T, crGL_SAMPLER_ADDRESS[sampler->addressV]);
#if !defined(CR_GLES_2)
		if(GL_TEXTURE_2D != gltarget) {
			glTexParameteri(gltarget, GL_TEXTURE_WRAP_R, crGL_SAMPLER_ADDRESS[sampler->addressW]);
		}
#endif
	}

	return CrTrue;
}

CrInputGpuFormatMapping CrInputGpuFormatMappings[] = {
	{CrGpuFormat_FloatR32,				1, GL_FLOAT, 0, sizeof(float)},
	{CrGpuFormat_FloatR32G32,			2, GL_FLOAT, 0, sizeof(float) * 2},
	{CrGpuFormat_FloatR32G32B32,		3, GL_FLOAT, 0, sizeof(float) * 3},
	{CrGpuFormat_FloatR32G32B32A32,	4, GL_FLOAT, 0, sizeof(float) * 4},
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

CR_API size_t crGenGpuInputId()
{
	return ++crContextImpl()->gpuInputId;
}

CR_API void crGpuProgramBindInput(CrGpuProgram* self, size_t gpuInputId, CrGpuProgramInput* inputs, size_t count)
{
	CrGpuProgramImpl* impl = (CrGpuProgramImpl*)self;

	size_t attri = 0;

#if !defined(CR_GLES_2)
	glBindVertexArray(impl->glVertexArray);
#endif

	for(attri=0; attri<count; ++attri) {
		CrGpuProgramInput* i = &inputs[attri];

		if(nullptr == i->buffer)
			continue;

		if(CrBufferType_Index == crBufferGetType(i->buffer)) {
			// bind index buffer
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ((CrBufferImpl*)i->buffer)->glName);
		}
		else if(CrBufferType_Vertex == crBufferGetType(i->buffer)) {
			// bind vertex buffer
			CrInputGpuFormatMapping* m = crInputGpuFormatMappingGet(i->format);

			int loc = glGetAttribLocation(impl->glName, i->name);

			if(nullptr != m && -1 != loc) {
				glBindBuffer(GL_ARRAY_BUFFER, ((CrBufferImpl*)i->buffer)->glName);
				glVertexAttribPointer(loc, m->elemCnt, m->elemType, m->normalized, i->stride, (void*)i->offset);
				glEnableVertexAttribArray(loc);
			}
		}
	}
}

static GLenum crGL_INDEX_TYPE[] = {
	GL_UNSIGNED_SHORT,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_INT,
};

CR_API void crGpuDrawPoint(size_t offset, size_t count)
{
	glDrawArrays(GL_POINTS, offset, count);
}

CR_API void crGpuDrawLine(size_t offset, size_t count, size_t flags)
{
	GLenum mode = (flags & CrGpuDraw_Stripped) ? GL_LINE_STRIP : GL_LINES;
	glDrawArrays(mode, offset, count);
}

CR_API void crGpuDrawLineIndexed(size_t offset, size_t count, size_t minIdx, size_t maxIdx, size_t flags)
{
	GLenum mode = (flags & CrGpuDraw_Stripped) ? GL_LINE_STRIP : GL_LINES;
	GLenum indexType = crGL_INDEX_TYPE[flags & 0x000F];
#if defined(CR_GLES_2)
	glDrawElements(mode, count, indexType, (void*)offset);
#else
	glDrawRangeElements(mode, minIdx, maxIdx, count, indexType, (void*)offset);
#endif
}

CR_API void crGpuDrawTriangle(size_t offset, size_t count, size_t flags)
{
	GLenum mode = (flags & CrGpuDraw_Stripped) ? GL_TRIANGLE_STRIP : GL_TRIANGLES;
	glDrawArrays(mode, offset, count);
}

CR_API void crGpuDrawTriangleIndexed(size_t offset, size_t count, size_t minIdx, size_t maxIdx, size_t flags)
{
	GLenum mode = (flags & CrGpuDraw_Stripped) ? GL_TRIANGLE_STRIP : GL_TRIANGLES;
	GLenum indexType = crGL_INDEX_TYPE[flags & 0x000F];
#if defined(CR_GLES_2)
	glDrawElements(mode, count, indexType, (void*)offset);
#else
	glDrawRangeElements(mode, minIdx, maxIdx, count, indexType, (void*)offset);
#endif
}

CR_API void crGpuDrawPatch(size_t offset, size_t count, size_t vertexPerPatch, size_t flags)
{
#if !defined(CR_GLES_2)
	GLenum mode = GL_PATCHES;
	if(nullptr != glPatchParameteri) {
		glPatchParameteri(GL_PATCH_VERTICES, vertexPerPatch);
		glDrawArrays(mode, offset, count);
	}
#endif
}

CR_API void crGpuDrawPatchIndexed(size_t offset, size_t count, size_t minIdx, size_t maxIdx, size_t vertexPerPatch, size_t flags)
{
#if !defined(CR_GLES_2)
	GLenum mode = GL_PATCHES;
	GLenum indexType = crGL_INDEX_TYPE[flags & 0x000F];
	if(nullptr != glPatchParameteri) {
		glPatchParameteri(GL_PATCH_VERTICES, vertexPerPatch);
		glDrawRangeElements(mode, minIdx, maxIdx, count, indexType, (void*)offset);
	}
#endif
}
