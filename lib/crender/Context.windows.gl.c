#include "Context.gl.h"
#include "Mem.h"

#define _WIN32_WINNT 0x0500
#define WINVER 0x0500
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <GL\wglew.h>
#include <stdio.h>

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

	self->i.apiName = "gl";
	self->i.apiMajorVer = 2;
	self->i.apiMinorVer = 1;
	self->i.xres = 853;
	self->i.yres = 480;

	crGpuStateInit(&self->i.gpuState);
	crFfpStateInit(&self->i.ffpState);
	memset(&self->appliedGpuState, 0, sizeof(CrGpuState));
	memset(&self->appliedFfpState, 0, sizeof(CrFfpState));

	return &self->i;
}

CR_API CrBool crContextInit(CrContext* self, void** window)
{
	CrContextImpl* impl = (CrContextImpl*)self;
	HWND* hWnd = (HWND*)window;
	WINDOWINFO info;
	
	GLenum err;
	PIXELFORMATDESCRIPTOR pfd;
	HDC hDC;
	HGLRC hRC;
	int pixelFormat;
	char glVersionStr[64];

	GetWindowInfo(*hWnd, &info);
	self->xres = info.rcClient.right - info.rcClient.left;
	self->yres = info.rcClient.bottom - info.rcClient.top;

	// Create the GL context.
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;

	hDC = GetDC(*hWnd);
	pixelFormat = ChoosePixelFormat(hDC, &pfd);

	SetPixelFormat(hDC, pixelFormat, &pfd);
	hRC = wglCreateContext(hDC);
	wglMakeCurrent(hDC, hRC);

	if (self->msaaLevel > 1)
	{
		int pixelAttribs[] =
		{
			WGL_SAMPLES_ARB, self->msaaLevel,
			WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_RED_BITS_ARB, 8,
			WGL_GREEN_BITS_ARB, 8,
			WGL_BLUE_BITS_ARB, 8,
			WGL_ALPHA_BITS_ARB, 8,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			0
		};
		int* sampleCount = pixelAttribs + 1;
		int* useSampleBuffer = pixelAttribs + 3;
		int pixelFormat = -1;
		PROC proc = wglGetProcAddress("wglChoosePixelFormatARB");
		unsigned int numFormats;
		PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC) proc;

		if (!wglChoosePixelFormatARB)
		{
			crDbgStr("Could not load function pointer for 'wglChoosePixelFormatARB'.  Is your driver properly installed?");
		}

		// Try fewer and fewer samples per pixel till we find one that is supported:
		while (pixelFormat <= 0 && *sampleCount >= 0)
		{
			wglChoosePixelFormatARB(hDC, pixelAttribs, 0, 1, &pixelFormat, &numFormats);
			(*sampleCount)--;
			if (*sampleCount <= 1)
			{
				*useSampleBuffer = GL_FALSE;
			}
		}

		// Win32 allows the pixel format to be set only once per app, so destroy and re-create the app:
		{
			
			char szName[64];
			int windowWidth, windowHeight, windowLeft, windowTop;
			
			GetClassName(*hWnd, szName, 64);

			windowWidth = info.rcWindow.right - info.rcWindow.left;
			windowHeight = info.rcWindow.bottom - info.rcWindow.top;
			windowLeft = info.rcWindow.left;
			windowTop = info.rcWindow.top;
			
			DestroyWindow(*hWnd);
			
			*hWnd = CreateWindowExA(info.dwExStyle, szName, szName, info.dwStyle, 
				windowLeft, windowTop, windowWidth, windowHeight,
				0, 0, 0, 0);

			SetWindowPos(*hWnd, HWND_TOP, windowLeft, windowTop, windowWidth, windowHeight, 0);
			hDC = GetDC(*hWnd);
			
			SetPixelFormat(hDC, pixelFormat, &pfd);
			hRC = wglCreateContext(hDC);
			wglMakeCurrent(hDC, hRC);
		}
	}

	err = glewInit();
	if (GLEW_OK != err) {
		crDbgStr("GLEW Error: %s\n", glewGetErrorString(err));
	}
	crDbgStr("OpenGL Version: %s\n", glGetString(GL_VERSION));

	if (!self->vsync)
		wglSwapIntervalEXT(0);

	sprintf(glVersionStr, "GL_VERSION_%d_%d", self->apiMajorVer, self->apiMinorVer);
	if ((self->apiMajorVer > 2) && glewIsSupported(glVersionStr))
	{
		const int contextAttribs[] =
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, self->apiMajorVer,
			WGL_CONTEXT_MINOR_VERSION_ARB, self->apiMinorVer,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
			0
		};

		GLuint vertexArray;

		HGLRC newRC = wglCreateContextAttribsARB(hDC, 0, contextAttribs);
		wglMakeCurrent(0, 0);
		wglDeleteContext(hRC);
		hRC = newRC;
		wglMakeCurrent(hDC, hRC);

		// 3.0 or above requires a vertex array
		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
	}

	impl->hDC = (GLuint)hDC;
	impl->hRC = (GLuint)hRC;
	impl->hwnd = *hWnd;

	// create FBOs
	if(nullptr == glGenFramebuffers) {
		crDbgStr("render target is not supported!\n");
	}
	else {
		impl->defFBOName = 0;
		glGenFramebuffers(1, &impl->rttFBOName);
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
	SwapBuffers((HDC)impl->hDC);
}

CR_API CrBool crContextChangeResolution(CrContext* self, size_t xres, size_t yres)
{
	CrContextImpl* impl = (CrContextImpl*)self;
	HWND hWnd = (HWND)impl->hwnd;
	RECT rcClient, rcWindow;
	POINT ptDiff;

	GetClientRect(hWnd, &rcClient);
	GetWindowRect(hWnd, &rcWindow);
	ptDiff.x = (rcWindow.right - rcWindow.left) - rcClient.right;
	ptDiff.y = (rcWindow.bottom - rcWindow.top) - rcClient.bottom;
	SetWindowPos(hWnd, 0, rcWindow.left, rcWindow.top, xres + ptDiff.x, yres + ptDiff.y, SWP_NOMOVE|SWP_NOZORDER);

	self->xres = xres;
	self->yres = yres;

	return CrTrue;
}