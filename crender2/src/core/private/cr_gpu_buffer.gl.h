#ifndef CR_GPU_BUFFER_GL_PRIVATE_H
#define CR_GPU_BUFFER_GL_PRIVATE_H

#include "cr_gpu_buffer.h"
#include "cr_gpu.gl.h"

namespace cr
{

struct gpu_buffer_gl : gpu_buffer
{
	CR_OVERRIDE_NEW_DELETE();

	gpu_gl* gpu;
	GLuint gl_name;
	GLenum gl_target;

	gpu_buffer_gl( context* ctx, gpu_gl* gpu );
	~gpu_buffer_gl( void );

	struct cmd_args
	{
		gpu_buffer_gl* self;
		cr_uint32 offset;
		cr_uint32 size;
		void* data;
		cr_gpu_callback callback;
	};

	static void create( cr_command_queue cmd_queue, cr_command_args args );
	static void update( cr_command_queue cmd_queue, cr_command_args args );
};

}	// namespace cr

#endif	// CR_GPU_BUFFER_WIN32_PRIVATE_H
