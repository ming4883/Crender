#ifndef CR_GPU_PROGRAM_GL_PRIVATE_H
#define CR_GPU_PROGRAM_GL_PRIVATE_H

#include "cr_gpu_program.h"
#include "cr_gpu.gl.h"

namespace cr
{

struct gpu_program_gl : gpu_program
{
	CR_OVERRIDE_NEW_DELETE();

	gpu_gl* gpu;
	GLuint gl_name;

	gpu_program_gl( context* ctx, gpu_gl* gpu );
	~gpu_program_gl( void );

	struct cmd_args
	{
		gpu_program_gl* self;
		cr_gpu_callback callback;
		cr_gpu_shader* shaders;
		cr_uint32 shader_count;
	};

	static void create( cr_command_queue cmd_queue, cr_command_args args );
};

}	// namespace cr

#endif	// CR_GPU_PROGRAM_GL_PRIVATE_H
