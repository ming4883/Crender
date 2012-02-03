#ifndef CR_GPU_WIN32_PRIVATE_H
#define CR_GPU_WIN32_PRIVATE_H

#include "cr_gpu.h"

#include <GL/glew.h>

namespace cr
{

struct win32_gpu : gpu
{
	GLuint hdc;
	GLuint hrc;
	void* hwnd;
	GLuint gl_vtx_array_name;

	static void _dstor( object* obj );

	void init( void** window, struct cr_gpu_desc* desc );
};

}	// namespace cr

#endif	// CR_GPU_WIN32_PRIVATE_H
