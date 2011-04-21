#include "Buffer.gl.h"
#include "Mem.h"

static GLenum crGL_BUFFER_TARGET[] = {
	GL_ARRAY_BUFFER,
	GL_ELEMENT_ARRAY_BUFFER,
	GL_ELEMENT_ARRAY_BUFFER,

#if !defined(CR_GLES_2)
	GL_ELEMENT_ARRAY_BUFFER,
	GL_UNIFORM_BUFFER,
#endif
};

#if !defined(CR_GLES_2)
static GLenum crGL_BUFFER_MAP_ACCESS[] = {
	GL_READ_ONLY,
	GL_WRITE_ONLY,
	GL_READ_WRITE,
};
#endif

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

	glDeleteBuffers(1, &impl->glName);

	crMem()->free(self, "CrBuffer");
}

CR_API CrBool crBufferInit(CrBuffer* self, CrBufferType type, size_t sizeInBytes, void* initialData)
{
	CrBufferImpl* impl = (CrBufferImpl*)self;

	self->sizeInBytes = sizeInBytes;
	self->type = type;

	glGenBuffers(1, &impl->glName);
	glBindBuffer(crGL_BUFFER_TARGET[self->type], impl->glName);
	glBufferData(crGL_BUFFER_TARGET[self->type], self->sizeInBytes, initialData, GL_STREAM_DRAW);

	self->flags = CrBuffer_Inited;

	return CrTrue;
}

CR_API void crBufferUpdate(CrBuffer* self, size_t offsetInBytes, size_t sizeInBytes, void* data)
{
	CrBufferImpl* impl = (CrBufferImpl*)self;

	if(nullptr == self)
		return;

	if(offsetInBytes + sizeInBytes > self->sizeInBytes)
		return;

	glBindBuffer(crGL_BUFFER_TARGET[self->type], impl->glName);
	glBufferSubData(crGL_BUFFER_TARGET[self->type], offsetInBytes, sizeInBytes, data);
}

CR_API void* crBufferMap(CrBuffer* self, CrBufferMapAccess access)
{
	CrBufferImpl* impl = (CrBufferImpl*)self;
	void* ret = nullptr;
	
	if(nullptr == self)
		return nullptr;

	if(0 != (self->flags & CrBuffer_Mapped))
		return nullptr;

#if !defined(CR_GLES_2)
	glBindBuffer(crGL_BUFFER_TARGET[self->type], impl->glName);
	ret = glMapBuffer(crGL_BUFFER_TARGET[self->type], crGL_BUFFER_MAP_ACCESS[access]);
	self->flags |= CrBuffer_Mapped;

#endif

	return ret;
}

CR_API void crBufferUnmap(CrBuffer* self)
{
	CrBufferImpl* impl = (CrBufferImpl*)self;

	if(nullptr == self)
		return;

	if(0 == (self->flags & CrBuffer_Mapped))
		return;

#if !defined(CR_GLES_2)
	glBindBuffer(crGL_BUFFER_TARGET[self->type], impl->glName);
	glUnmapBuffer(crGL_BUFFER_TARGET[self->type]);

	self->flags &= ~CrBuffer_Mapped;
#endif
}
