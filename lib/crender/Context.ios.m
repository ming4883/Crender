#include "Context.gl.h"
#include "Mem.h"

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

CrContextImpl* current = nullptr;

CR_API CrContext* crContext()
{
	return &current->i;
}

CR_API CrContextImpl* crContextImpl()
{
	return current;
}

CR_API CrBool crContextFixedPipelineOnly()
{
	return 1 == current->i.apiMajorVer;
}

CR_API CrContext* crContextAlloc()
{
	CrContextImpl* self = crMem()->alloc(sizeof(CrContextImpl), "CrContext");
	memset(self, 0, sizeof(CrContextImpl));

	self->i.apiName = "gles";
	self->i.apiMajorVer = 2;
	self->i.apiMinorVer = 0;
	self->i.xres = 480;
	self->i.yres = 320;

	return &self->i;
}

CR_API CrBool crContextInit(CrContext* self, void** window)
{
	CrContextImpl* impl = (CrContextImpl*)self;

	CAEAGLLayer** eaglLayer = (CAEAGLLayer**)window;

	EAGLRenderingAPI api = kEAGLRenderingAPIOpenGLES2;
	if(self->apiMajorVer == 1)
		api = kEAGLRenderingAPIOpenGLES1;

	EAGLContext* context = [[EAGLContext alloc] initWithAPI:api];

	if(!context) {
		context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
		self->apiMajorVer = 1;
		crDbgStr("fall back to gles1.0");
	}

	if (!context || ![EAGLContext setCurrentContext:context])
		return CrFalse;

	impl->eaglContext = (GLuint)context;

	// default FBO
	glGenFramebuffers(1, &impl->defFBOName);
	glBindFramebuffer(GL_FRAMEBUFFER, impl->defFBOName);

	// default Color buffer
	glGenRenderbuffers(1, &impl->defColorBufName);
	glBindRenderbuffer(GL_RENDERBUFFER, impl->defColorBufName);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, impl->defColorBufName);

	// default Color buffer
	glGenRenderbuffers(1, &impl->defDepthBufName);
	glBindRenderbuffer(GL_RENDERBUFFER, impl->defDepthBufName);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, impl->defDepthBufName);

	// color buffer storage
	glBindRenderbuffer(GL_RENDERBUFFER, impl->defColorBufName);
    [context renderbufferStorage:GL_RENDERBUFFER fromDrawable:*eaglLayer];
	int w, h;
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &w);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &h);
	self->xres = w;
	self->yres = h;

	// depth buffer storage
	glBindRenderbuffer(GL_RENDERBUFFER, impl->defDepthBufName);
	glRenderbufferStorage(GL_RENDERBUFFER,GL_DEPTH_COMPONENT16, self->xres, self->yres);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        crDbgStr("Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }

	current = impl;

	return CrTrue;
}

CR_API void crContextFree(CrContext* self)
{
	current = nullptr;

	crMem()->free(self, "CrContext");
}

CR_API CrBool crContextPreRender(CrContext* self)
{
	return CrTrue;
}

CR_API void crContextPostRender(CrContext* self)
{
}

CR_API void crContextSwapBuffers(CrContext* self)
{
	CrContextImpl* impl = (CrContextImpl*)self;

	EAGLContext* context = (EAGLContext*)impl->eaglContext;

	glBindRenderbuffer(GL_RENDERBUFFER, impl->defColorBufName);
    [context presentRenderbuffer:GL_RENDERBUFFER];
}
