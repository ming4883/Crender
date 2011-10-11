#ifndef __CRENDER_CONTEXT_GL_H__
#define __CRENDER_CONTEXT_GL_H__

#include "Context.h"

#ifdef CR_ANDROID
#	include <GLES2/gl2.h>
#	include <GLES2/gl2ext.h>
#	include <EGL/egl.h>
#	define CR_GLES_2

#elif defined(CR_APPLE_IOS)
#	include <OpenGLES/ES2/gl.h>
#	include <OpenGLES/ES2/glext.h>
#	include <OpenGLES/ES1/gl.h>
#	include <OpenGLES/ES1/glext.h>
#	define CR_GLES_2

#else
#	include <GL/glew.h>
#endif

#if defined(CR_GLES_2)
#	define glClearDepth glClearDepthf
#	define glDepthRange glDepthRangef
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CrContextImpl
{
	CrContext i;

	CrGpuState appliedGpuState;
	CrFfpState appliedFfpState;

	unsigned int gpuInputId;
	GLuint defFBOName;
	GLuint rttFBOName;

#if defined(CR_WIN32)
	GLuint hDC;
	GLuint hRC;
	void* hwnd;
#endif

#if defined(CR_APPLE_IOS)
	GLuint eaglContext;
	GLuint defColorBufName;
	GLuint defDepthBufName;
#endif

#if defined(CR_ANDROID)
	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
#endif

} CrContextImpl;

CR_API CrContextImpl* crContextImpl();
CR_API void crGpuStateInit(CrGpuState* self);
CR_API void crFfpStateInit(CrFfpState* self);

#define crCheckGLError() {GLenum err = glGetError(); if(GL_NO_ERROR != err) crDbgStr("unhandled GL error 0x%04x before %s %d\n", err, __FILE__, __LINE__); }

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_CONTEXT_GL_H__
