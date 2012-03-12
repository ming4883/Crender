#ifndef CR_GPU_BUFFER_GL_PRIVATE_H
#define CR_GPU_BUFFER_GL_PRIVATE_H

#include "cr_gpu_shader.h"
#include "cr_gpu.gl.h"

namespace cr
{

struct gpu_shader_gl : gpu_shader
{
	CR_OVERRIDE_NEW_DELETE();

	gpu_gl* gpu;
	GLuint gl_name;
	GLenum gl_target;

	gpu_shader_gl( context* ctx, gpu_gl* gpu );
	~gpu_shader_gl( void );

	struct cmd_args
	{
		gpu_shader_gl* self;
		cr_uint32 offset;
		cr_uint32 size;
		void* data;
	};

	static void create( cr_command_queue cmd_queue, cr_command_args args );
	static void update( cr_command_queue cmd_queue, cr_command_args args );
};

}	// namespace cr

#endif	// CR_GPU_BUFFER_WIN32_PRIVATE_H