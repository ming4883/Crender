#include "cr_gpu_shader.gl.h"
#include "cr_context.h"

namespace cr
{

gpu_shader_gl::gpu_shader_gl( context* ctx, gpu_gl* g )
	: gpu_shader( ctx )
	, gpu( g )
	, gl_name( 0 )
	, gl_target( 0 )
{
}

gpu_shader_gl::~gpu_shader_gl( void )
{
}

void gpu_shader_gl::create( cr_command_queue cmd_queue, cr_command_args a )
{
	static GLenum gl_targets[] =
	{
		GL_ARRAY_BUFFER,
		GL_ELEMENT_ARRAY_BUFFER,
		GL_ELEMENT_ARRAY_BUFFER,
		0, // not supported
		0, // not supported
	};

	cmd_args* args = ( cmd_args* )a;

	gpu_shader_gl* self =  args->self;

	if ( self->is_sys_mem() )
	{
		self->sys_mem = cr_mem_alloc( self->size );
	}
	else
	{
		self->gl_target = gl_targets[ self->type & CR_GPU_BUFFER_MASK ];

		glGenBuffers( 1, &self->gl_name );
		glBindBuffer( self->gl_target, self->gl_name );
		glBufferData( self->gl_target, self->size, nullptr, GL_STREAM_DRAW );

		cr_check_gl_err();
	}
}

void gpu_shader_gl::update( cr_command_queue cmd_queue, cr_command_args a )
{
	cmd_args* args = ( cmd_args* )a;

	gpu_shader_gl* self =  args->self;

	if ( ( args->offset + args->size ) <= self->size )
	{
		if ( self->is_sys_mem() )
		{
			memcpy( ( ( char* ) self->sys_mem ) + args->offset, args->data, args->size );
		}
		else
		{
			glBindBuffer( self->gl_target, self->gl_name );
			glBufferSubData( self->gl_target, args->offset, args->size, args->data );

			cr_check_gl_err();
		}
	}
}

}

#ifdef __cplusplus
extern "C" {
#endif

	typedef cr::gpu_shader_gl gpu_shader_t;

	CR_API cr_gpu_shader cr_gpu_shader_new( cr_context context, cr_gpu gpu, enum cr_gpu_shader_type type, cr_uint32 size )
	{
		typedef gpu_shader_t::cmd_args args_t;
		cr_assert( CR_COMMAND_ARGS_SIZE >= sizeof( args_t ) );
		
		cr_assert( cr::context::singleton );

		gpu_shader_t* self = new gpu_shader_t( cr_context_get( context ), ( cr::gpu_gl* )gpu );
		self->size = size;
		self->type = type;

		args_t* args = nullptr;
		cr_command_queue_produce( self->gpu->feeding_queue->cmd_queue, ( cr_command_args* )&args, gpu_shader_t::create );

		args->self = self;

		return ( cr_gpu_shader )self;
	}

	CR_API void cr_gpu_shader_update( cr_gpu_shader s, cr_uint32 offset, cr_uint32 size, void* data )
	{
		typedef gpu_shader_t::cmd_args args_t;
		cr_assert( CR_COMMAND_ARGS_SIZE >= sizeof( args_t ) );

		gpu_shader_t* self = ( gpu_shader_t* )s;

		args_t* args = nullptr;
		cr_command_queue_produce( self->gpu->feeding_queue->cmd_queue, ( cr_command_args* )&args, gpu_shader_t::update );

		args->self = self;
		args->offset = offset;
		args->size = size;
		args->data = cr_mem_alloc( size );

		memcpy( args->data, data, size );
	}

#ifdef __cplusplus
}
#endif