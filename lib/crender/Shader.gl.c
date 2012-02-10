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

	crTrace("crGpuShaderInit");

	if(self->flags & CrGpuShader_Inited) {
		crDbgStr("CrGpuShader already inited!\n");
		return CrFalse;
	}

	crCheckGLError();	// clear any unhandled gl errors

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

	crCheckGLError();

	return CrTrue;
}

CR_API void crGpuShaderFree(CrGpuShader* self)
{
	CrGpuShaderImpl* impl = (CrGpuShaderImpl*)self;

	crTrace("crGpuShaderFree");

	if(nullptr == self)
		return;

	glDeleteShader(impl->glName);
	crMem()->free(self, "CrGpuShader");

	crCheckGLError();
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
	GLint uniformCnt;
	GLint attrCnt;
	CrGpuProgramImpl* impl = (CrGpuProgramImpl*)self;

	crTrace("crGpuProgramInit");

	if(self->flags & CrGpuProgram_Inited) {
		crDbgStr("CrGpuProgram already inited!\n");
		return CrFalse;
	}

	crCheckGLError();	// clear any unhandled gl errors

	crTrace("glCreateProgram");
	impl->glName = glCreateProgram();
	crCheckGLError();

	// attach shaders
	crTrace("glAttachShader");
	for(i=0; i<shaderCnt; ++i) {
		if(nullptr != shaders[i]) {
			glAttachShader(impl->glName, ((CrGpuShaderImpl*)shaders[i])->glName);
		}
	}
	{	GLenum glerr = glGetError();
		if(GL_INVALID_OPERATION == glerr) {
			crDbgStr("GL_INVALID_OPERATION\n");
			if(GL_FALSE == glIsProgram(impl->glName))
				crDbgStr("0x%08x is not a program\n", impl->glName);
			crDbgStr("code:\n");
			for(i=0; i<shaderCnt; ++i) {
				if(nullptr == shaders[i]) continue;
				crDbgStr("  -0x%08x\n", ((CrGpuShaderImpl*)shaders[i])->glName);
			}
			{	GLsizei cnt;
				GLuint shaders[16];
				glGetAttachedShaders(impl->glName, 16, &cnt, shaders);

				crDbgStr("gl:\n");
				for(i=0; i<(size_t)cnt; ++i) {
					crDbgStr("  -0x%08x\n", shaders[i]);
				}
			}
		}
		else if(GL_INVALID_VALUE == glerr) {
			crDbgStr("GL_INVALID_VALUE\n");
		}
	}

	// link program
	crTrace("glLinkProgram");
	glLinkProgram(impl->glName);
	crCheckGLError();

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

	crTrace("glUseProgram");
	glUseProgram(impl->glName);
	crCheckGLError();

	// query all uniforms
	crTrace("glGetActiveUniform");
	{
		GLint i;
		GLsizei uniformLength;
		GLint uniformSize;
		GLenum uniformType;
		char uniformName[32];
		GLuint texunit = 0;

		glGetProgramiv(impl->glName, GL_ACTIVE_UNIFORMS, &uniformCnt);

		impl->uniforms = crMem()->alloc(sizeof(CrGpuProgramUniform) * uniformCnt, "CrGpuProgram");
		memset(impl->uniforms, 0, sizeof(CrGpuProgramUniform) * uniformCnt);

		for(i=0; i<uniformCnt; ++i) {
			size_t c;
			CrGpuProgramUniform* uniform;
			glGetActiveUniform(impl->glName, i, crCountOf(uniformName), &uniformLength, &uniformSize, &uniformType, uniformName);
			for(c=0; c<sizeof(uniformName); ++c) {
				// for array uniform, some driver may return var_name[0] :-<
				if(uniformName[c] == '[') {
					uniformName[c] = '\0';
					break;
				}
			}
			uniform = &impl->uniforms[i];
			uniform->hash = CrHash(uniformName);
			uniform->loc = glGetUniformLocation(impl->glName, uniformName);
			uniform->size = uniformSize;
			uniform->texunit = texunit;

			HASH_ADD_INT(impl->uniformCache, hash, uniform);

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
						glUniform1i(uniform->loc, texunit++);
					}
					break;
				default:
					uniform->texunit = -1;
					break;
			}
			//crDbgStr("%s %d %d 0x%04x %d\n", uniformName, i, uniformSize, uniformType, uniform->texunit);
		}
	}
	crCheckGLError();

	// query all attributes
	crTrace("glGetActiveAttrib");
	{
		GLint i;
		GLsizei attrLength;
		GLint attrSize;
		GLenum attrType;
		char attrName[32];

		glGetProgramiv(impl->glName, GL_ACTIVE_ATTRIBUTES, &attrCnt);

		impl->attrs = crMem()->alloc(sizeof(CrGpuProgramAttribute) * attrCnt, "CrGpuProgram");
		memset(impl->attrs, 0, sizeof(CrGpuProgramAttribute) * attrCnt);

		for(i=0; i<attrCnt; ++i) {
			size_t c;
			CrGpuProgramAttribute* attr;
			glGetActiveAttrib(impl->glName, i, crCountOf(attrName), &attrLength, &attrSize, &attrType, attrName);
			for(c=0; c<sizeof(attrName); ++c) {
				// for array uniform, some driver may return var_name[0] :-<
				if(attrName[c] == '[') {
					attrName[c] = '\0';
					break;
				}
			}
			attr = &impl->attrs[i];
			attr->hash = CrHash(attrName);
			attr->loc = glGetAttribLocation(impl->glName, attrName);

			HASH_ADD_INT(impl->attrCache, hash, attr);

			//crDbgStr("%s %d %d 0x%04x %d\n", uniformName, i, uniformSize, uniformType, uniform->texunit);
		}
	}
	crCheckGLError();

	crDbgStr("glProgram %d has %d uniforms %d attributes\n", impl->glName, uniformCnt, attrCnt);

#if !defined(CR_GLES_2)
	//glGenVertexArrays(1, &impl->glVertexArray);
#endif

	return CrTrue;

}

CR_API void crGpuProgramFree(CrGpuProgram* self)
{
	CrGpuProgramImpl* impl = (CrGpuProgramImpl*)self;

	crTrace("crGpuProgramFree");

	if(nullptr == self)
		return;

	HASH_CLEAR(hh, impl->uniformCache);
	crMem()->free(impl->uniforms, "CrGpuProgram");

	HASH_CLEAR(hh, impl->attrCache);
	crMem()->free(impl->attrs, "CrGpuProgram");

	glDeleteProgram(impl->glName);

#if !defined(CR_GLES_2)
	//glDeleteVertexArrays(1, &impl->glVertexArray);
#endif

	crMem()->free(self, "CrGpuProgram");

	crCheckGLError();
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

	crCheckGLError();	// clear any unhandled gl errors

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

	HASH_FIND_INT(impl->uniformCache, &hash, uniform);
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

	HASH_FIND_INT(impl->uniformCache, &hash, uniform);
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

	HASH_FIND_INT(impl->uniformCache, &hash, uniform);
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

	HASH_FIND_INT(impl->uniformCache, &hash, uniform);
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

	HASH_FIND_INT(impl->uniformCache, &hash, uniform);
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

	HASH_FIND_INT(impl->uniformCache, &hash, uniform);
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

char* crGpuFixedIndexPtr = nullptr;

GLuint lastAttrLoc[16] = {0};
size_t lastAttrCnt = 0;

CR_API void crGpuBindProgramInput(CrGpuProgram* self, size_t gpuInputId, CrGpuProgramInput* inputs, size_t count)
{
	CrGpuProgramImpl* impl = (CrGpuProgramImpl*)self;

	size_t attri = 0;

#if !defined(CR_GLES_2)
	//glBindVertexArray(impl->glVertexArray);
#endif

	// disable pervious inputs
	for(attri=0; attri<lastAttrCnt; ++attri) {
		glDisableVertexAttribArray(lastAttrLoc[attri]);
	}
	lastAttrCnt = 0;

	// enable current inputs
	for(attri=0; attri<count; ++attri) {
		CrGpuProgramInput* i = &inputs[attri];

		if(nullptr == i->buffer)
			continue;

		if(CrBufferType_Index == crBufferGetType(i->buffer)) {
			// bind index buffer
			if(i->buffer->type & CrBufferType_SysMem) {
				crGpuFixedIndexPtr = (char*)i->buffer->sysMem;
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
			}
			else {
				crGpuFixedIndexPtr = nullptr;
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ((CrBufferImpl*)i->buffer)->glName);
			}
		}
		else if(CrBufferType_Vertex == crBufferGetType(i->buffer)) {
			// bind vertex buffer
			CrInputGpuFormatMapping* m = crInputGpuFormatMappingGet(i->format);
			CrGpuProgramAttribute* a = nullptr;

			if(0 == i->nameHash)
				i->nameHash = CrHash(i->name);
			HASH_FIND_INT(impl->attrCache, &i->nameHash, a);

			if(nullptr != a) {

				if(i->buffer->type & CrBufferType_SysMem) {
					glBindBuffer(GL_ARRAY_BUFFER, 0);
					glVertexAttribPointer(a->loc, m->elemCnt, m->elemType, m->normalized, i->stride, (char*)i->buffer->sysMem + i->offset);
					glEnableVertexAttribArray(a->loc);
				}
				else {
					glBindBuffer(GL_ARRAY_BUFFER, ((CrBufferImpl*)i->buffer)->glName);
					glVertexAttribPointer(a->loc, m->elemCnt, m->elemType, m->normalized, i->stride, (void*)i->offset);
					glEnableVertexAttribArray(a->loc);
				}

				lastAttrLoc[lastAttrCnt++] = a->loc;
			}
		}
	}
}

CR_API void crGpuBindFixedInput(size_t gpuInputId, CrGpuFixedInput* input)
{
#ifndef CR_ANDROID
	if(input->index.buffer) {
		CrGpuProgramInput* i = &input->index;
		crGpuFixedIndexPtr = (char*)i->buffer->sysMem;
	}
	else {
		crGpuFixedIndexPtr = nullptr;
	}

	if(input->position.buffer) {
		CrGpuProgramInput* i = &input->position;
		CrInputGpuFormatMapping* m = crInputGpuFormatMappingGet(i->format);
		if(nullptr != m) {
			glVertexPointer(m->elemCnt, m->elemType, i->stride, (char*)i->buffer->sysMem + i->offset);
			glEnableClientState(GL_VERTEX_ARRAY);
		}
	}
	else {
		glDisableClientState(GL_VERTEX_ARRAY);
	}

	if(input->normal.buffer) {
		CrGpuProgramInput* i = &input->normal;
		CrInputGpuFormatMapping* m = crInputGpuFormatMappingGet(i->format);
		if(nullptr != m) {
			glNormalPointer(m->elemType, i->stride, (char*)i->buffer->sysMem + i->offset);
			glEnableClientState(GL_NORMAL_ARRAY);
		}
	}
	else {
		glDisableClientState(GL_NORMAL_ARRAY);
	}

	if(input->color.buffer) {
		CrGpuProgramInput* i = &input->color;
		CrInputGpuFormatMapping* m = crInputGpuFormatMappingGet(i->format);
		if(nullptr != m) {
			glColorPointer(m->elemCnt, m->elemType, i->stride, (char*)i->buffer->sysMem + i->offset);
			glEnableClientState(GL_COLOR_ARRAY);
		}
	}
	else {
		glDisableClientState(GL_COLOR_ARRAY);
	}

	if(input->texcoord.buffer) {
		CrGpuProgramInput* i = &input->texcoord;
		CrInputGpuFormatMapping* m = crInputGpuFormatMappingGet(i->format);
		if(nullptr != m) {
			glActiveTexture(GL_TEXTURE0);
			glBindBuffer(GL_ARRAY_BUFFER, ((CrBufferImpl*)i->buffer)->glName);
			glTexCoordPointer(m->elemCnt, m->elemType, i->stride, (char*)i->buffer->sysMem + i->offset);
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		}
	}
	else {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
#endif
}

CR_API void crGpuBindFixedTexture(size_t unit, struct CrTexture* texture, const struct CrSampler* sampler)
{
	glActiveTexture(GL_TEXTURE0 + unit);
	if(nullptr == texture) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else {
		int gltarget = ((CrTextureImpl*)texture)->glTarget;
		int glname = ((CrTextureImpl*)texture)->glName;
		glEnable(gltarget);
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
}

static GLenum crGL_INDEX_TYPE[] = {
	GL_UNSIGNED_SHORT,
	GL_UNSIGNED_BYTE,
	GL_UNSIGNED_INT,
};

static size_t crGL_INDEX_SIZE[] = {
	sizeof(short),
	sizeof(char),
	sizeof(int),
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
	size_t byteOffset = offset * crGL_INDEX_SIZE[flags & 0x000F];

	if(nullptr != crGpuFixedIndexPtr) {
		byteOffset += (size_t)crGpuFixedIndexPtr;
	}
#if defined(CR_GLES_2)
		glDrawElements(mode, count, indexType, (void*)byteOffset);
#else
		glDrawRangeElements(mode, minIdx, maxIdx, count, indexType, (void*)byteOffset);
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
	size_t byteOffset = offset * crGL_INDEX_SIZE[flags & 0x000F];

	if(nullptr != crGpuFixedIndexPtr) {
		byteOffset += (size_t)crGpuFixedIndexPtr;
	}
#if defined(CR_GLES_2)
		glDrawElements(mode, count, indexType, (void*)byteOffset);
#else
		glDrawRangeElements(mode, minIdx, maxIdx, count, indexType, (void*)byteOffset);
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
	size_t byteOffset = offset * crGL_INDEX_SIZE[flags & 0x000F];
	if(nullptr != glPatchParameteri) {
		glPatchParameteri(GL_PATCH_VERTICES, vertexPerPatch);
		glDrawRangeElements(mode, minIdx, maxIdx, count, indexType, (void*)byteOffset);
	}
#endif
}
