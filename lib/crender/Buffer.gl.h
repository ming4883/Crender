#ifndef __CRENDER_BUFFER_GL_H__
#define __CRENDER_BUFFER_GL_H__

#include "API.gl.h"
#include "Buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CrBufferImpl
{
	CrBuffer i;

	GLuint glName;

} CrBufferImpl;

#ifdef __cplusplus
}
#endif

#endif	// __CRENDER_BUFFER_GL_H__
