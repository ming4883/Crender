#ifndef CR_GPU_GL_PRIVATE_H
#define CR_GPU_GL_PRIVATE_H

#include "cr_gpu.h"
#include "cr_command_queue.h"

#include <GL/glew.h>

#if defined( CR_DEBUG )
#	define cr_check_gl_err() \
	{ GLenum err = glGetError(); if ( GL_NO_ERROR != err ) cr_printf( "GL error 0x%04x before %s %d\n", err, __FILE__, __LINE__ ); }
#else
#	define cr_check_gl_err()
#endif

namespace cr
{

struct gpu_gl : gpu
{
	CR_OVERRIDE_NEW_DELETE();

	GLuint hdc;
	GLuint hrc;
	void* hwnd;
	GLuint gl_vtx_array_name;

	struct queue
	{
		enum status_e
		{
			EMPTY,
			FEEDING,
			FULL,
		};

		cr_command_queue cmd_queue;
		status_e status;
	};

	enum {CMD_QUEUE_COUNT = 2};

	queue queues[CMD_QUEUE_COUNT];
	queue* feeding_queue;

	gpu_gl( context* ctx );
	~gpu_gl( void );

	void init( void** window, struct cr_gpu_desc* desc );

// command related
	struct cmd_args
	{
		gpu_gl* self;
		float value[16];
	};

	static void swap_buffer( cr_command_queue cmd_queue, cr_command_args args );
	static void set_viewport( cr_command_queue cmd_queue, cr_command_args args );
	static void clear_color( cr_command_queue cmd_queue, cr_command_args args );
	static void clear_depth( cr_command_queue cmd_queue, cr_command_args args );
};

}	// namespace cr

#endif	// CR_GPU_GL_PRIVATE_H
