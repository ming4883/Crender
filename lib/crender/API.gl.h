#ifndef __CRENDER_API_GL_H__
#define __CRENDER_API_GL_H__

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

typedef struct CrApiPrivates
{
	unsigned int gpuInputId;
#if defined(CR_APPLE_IOS)
	GLuint defFBOName;
	GLuint defColorBufName;
	GLuint defDepthBufName;
#endif
} CrApiPrivates;

extern CrApiPrivates crAPI;

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_API_GL_H__
