#include "Context.gl.h"
#include "Mem.h"

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
	self->i.xres = 800;
	self->i.yres = 480;
	
	crGpuStateInit(&self->i.gpuState);
	crFfpStateInit(&self->i.ffpState);

	return &self->i;
}

CR_API CrBool crContextInit(CrContext* self, void** window)
{
	CrContextImpl* impl = (CrContextImpl*)self;
	
	const EGLint configAttribs[] = {
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_BLUE_SIZE, 5,
			EGL_GREEN_SIZE, 6,
			EGL_RED_SIZE, 5,
			EGL_DEPTH_SIZE, 16,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_NONE
	};
	
	const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2,	EGL_NONE};
	
	EGLint w, h, format;
	EGLint numConfigs;
	EGLConfig config;
	ANativeWindow* anwindow = (ANativeWindow*)*window;
	
	impl->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	eglInitialize(impl->display, 0, 0);
	eglChooseConfig(impl->display, configAttribs, &config, 1, &numConfigs);
	
	if (0 == numConfigs) {
		crDbgStr("eglChooseConfig return %d configs", numConfigs);
		return CrFalse;
	}

	eglGetConfigAttrib(impl->display, config, EGL_NATIVE_VISUAL_ID, &format);
	ANativeWindow_setBuffersGeometry(anwindow, 0, 0, format);

	impl->surface = eglCreateWindowSurface(impl->display, config, anwindow, nullptr);
	impl->context = eglCreateContext(impl->display, config, EGL_NO_CONTEXT, contextAttribs);

	if (eglMakeCurrent(impl->display, impl->surface, impl->surface, impl->context) == EGL_FALSE) {
		crDbgStr("Unable to eglMakeCurrent");
		return CrFalse;
	}

	eglQuerySurface(impl->display, impl->surface, EGL_WIDTH, &w);
	eglQuerySurface(impl->display, impl->surface, EGL_HEIGHT, &h);

	self->xres = w;
	self->yres = h;
	
	current = impl;

	return CrTrue;
}

CR_API void crContextFree(CrContext* self)
{
	CrContextImpl* impl = (CrContextImpl*)self;
	
	current = nullptr;
	
	if (impl->display != EGL_NO_DISPLAY) {
		eglMakeCurrent(impl->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (impl->context != EGL_NO_CONTEXT) {
			eglDestroyContext(impl->display, impl->context);
		}
		if (impl->surface != EGL_NO_SURFACE) {
			eglDestroySurface(impl->display, impl->surface);
		}
		eglTerminate(impl->display);
	}
	impl->display = EGL_NO_DISPLAY;
	impl->context = EGL_NO_CONTEXT;
	impl->surface = EGL_NO_SURFACE;

	crMem()->free(self, "CrContext");
}

CR_API CrBool crContextPreRender(CrContext* self)
{
	return CrTrue;
}

CR_API void crContextPostRender(CrContext* self)
{
	GLint err = glGetError();
	if(err != GL_NO_ERROR) {
		crDbgStr("GL has error %x", err);
	}
}

CR_API void crContextSwapBuffers(CrContext* self)
{
	CrContextImpl* impl = (CrContextImpl*)self;
	eglSwapBuffers(impl->display, impl->surface);
}

CR_API CrBool crContextChangeResolution(CrContext* self, size_t xres, size_t yres)
{
	return CrFalse;
}