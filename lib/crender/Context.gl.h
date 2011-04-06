#ifndef __CRENDER_CONTEXT_GL_H__
#define __CRENDER_CONTEXT_GL_H__

#include "Context.h"

#ifdef CR_ANDROID
#	include <GLES2/gl2.h>
#	include <GLES2/glext.h>
#	define CR_GLES_2

#elif defined(CR_APPLE_IOS)
#	include <OpenGLES/ES2/gl.h>
#	include <OpenGLES/ES2/glext.h>
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

	unsigned int gpuInputId;

#if defined(CR_WIN32)
	GLuint hDC;
    GLuint hRC;
#endif

#if defined(CR_APPLE_IOS)
	GLuint eaglContext;
	GLuint defFBOName;
	GLuint defColorBufName;
	GLuint defDepthBufName;
#endif

} CrContextImpl;

CR_API CrContextImpl* crContextImpl();

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_CONTEXT_GL_H__