#include "RenderTarget.gl.h"
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
			crMem()->free(it, "CrRenderTarget");
		}

		glDeleteFramebuffers(1, &impl->glName);
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

	glGenFramebuffers(1, &impl->glName);
	
	self->flags |= CrRenderTarget_Inited;
}

CR_API CrRenderBuffer* crRenderTargetAcquireBuffer(CrRenderTarget* self, size_t width, size_t height, CrGpuFormat format)
{
	CrRenderTargetImpl* impl = (CrRenderTargetImpl*)self;

	CrRenderBufferImpl* buffer;
	CrRenderBufferImpl* it;
	LL_FOREACH(impl->bufferList, it) {
		CrTexture* tex = it->i.texture;
		if(!it->acquired && (width == tex->width) && (height == tex->height) && (format == tex->format)) {
			return &it->i;
		}
	}

	buffer = crMem()->alloc(sizeof(CrRenderBufferImpl), "CrRenderTarget");
	buffer->acquired = CrTrue;
	buffer->i.texture = crTextureAlloc();
	crTextureInitRtt(buffer->i.texture, width, height, 0, 1, format);

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

static GLenum crGL_ATTACHMENT_POINT[] =
{	GL_COLOR_ATTACHMENT0,
#if !defined(CR_GLES_2)
	GL_COLOR_ATTACHMENT1,
	GL_COLOR_ATTACHMENT2,
	GL_COLOR_ATTACHMENT3,
	GL_COLOR_ATTACHMENT4,
	GL_COLOR_ATTACHMENT5,
	GL_COLOR_ATTACHMENT6,
	GL_COLOR_ATTACHMENT7,
#endif
};

CR_API void crRenderTargetPreRender(CrRenderTarget* self, CrRenderBuffer** colors, CrRenderBuffer* depth)
{
	CrRenderTargetImpl* impl = (CrRenderTargetImpl*)self;

	size_t bufCnt;
	CrRenderBuffer** curr;
	if(nullptr == self) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, impl->glName);
	
	// attach color buffers
	bufCnt = 0;
	if(nullptr != colors) {
		curr = (CrRenderBuffer**)colors;
		while(*curr != nullptr) {
			CrTexture* tex = (*curr)->texture;
			glFramebufferTexture2D(GL_FRAMEBUFFER, crGL_ATTACHMENT_POINT[bufCnt], ((CrTextureImpl*)tex)->glTarget, ((CrTextureImpl*)tex)->glName, 0);
			++curr;
			++bufCnt;
		}
	}

	// attach depth buffers
	if(depth != nullptr) {
		CrTexture* tex = depth->texture;
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, ((CrTextureImpl*)tex)->glTarget, ((CrTextureImpl*)tex)->glName, 0);
	}

#if !defined(CR_GLES_2)
	// assign buffer bindings
	glDrawBuffers(bufCnt, crGL_ATTACHMENT_POINT);
#endif
	{	// check for framebuffer's complete status
		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if(GL_FRAMEBUFFER_COMPLETE != status) {
			crDbgStr("imcomplete framebuffer status: %x\n", status);
		}
	}
}

CR_API void crRenderTargetSetViewport(float x, float y, float w, float h, float zmin, float zmax)
{
	glViewport((GLint)x, (GLint)y, (GLsizei)w, (GLsizei)h);
	glDepthRange(zmin, zmax);
}

CR_API void crRenderTargetClearColor(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT);
}

CR_API void crRenderTargetClearDepth(float z)
{
	glClearDepth(z);
	glClear(GL_DEPTH_BUFFER_BIT);
}
