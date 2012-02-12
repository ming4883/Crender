#ifndef CR_GPU_WIN32_PRIVATE_H
#define CR_GPU_WIN32_PRIVATE_H

#include "cr_gpu.h"
#include "cr_command_queue.h"

#include <GL/glew.h>

namespace cr
{

struct win32_gpu : gpu
{
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

	static void _dstor( object* obj );

	void init( void** window, struct cr_gpu_desc* desc );

	struct cmd_args
	{
		win32_gpu* self;
	};

	struct float16_args : cmd_args
	{
		float value[16];
	};

	static void swap_buffer( cr_command_queue cmd_queue, void* arg );
	static void set_viewport( cr_command_queue cmd_queue, void* arg );
	static void clear_color( cr_command_queue cmd_queue, void* arg );
	static void clear_depth( cr_command_queue cmd_queue, void* arg );
};

}	// namespace cr

#endif	// CR_GPU_WIN32_PRIVATE_H
