#include "cr_gpu_buffer.gl.h"
#include "cr_context.h"

namespace cr
{

gpu_buffer_gl::gpu_buffer_gl( context* ctx, gpu_gl* g )
	: gpu_buffer( ctx )
	, gpu( g )
	, gl_name( 0 )
	, gl_target( 0 )
{
	cr_retain( ( cr_object )gpu );
}

gpu_buffer_gl::~gpu_buffer_gl( void )
{
	if ( is_sys_mem() )
	{
		cr_mem_free( sys_mem );
	}
	else
	{
		glDeleteBuffers( 1, &gl_name );

		cr_check_gl_err();
	}

	cr_release( ( cr_object )gpu );
}

void gpu_buffer_gl::create( cr_command_queue cmd_queue, cr_command_args a )
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

	gpu_buffer_gl* self =  args->self;

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

void gpu_buffer_gl::update( cr_command_queue cmd_queue, cr_command_args a )
{
	cmd_args* args = ( cmd_args* )a;

	gpu_buffer_gl* self =  args->self;

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

		if ( args->callback.func )
		{
			args->callback.func( ( cr_object )self, 0, args->callback.args );
		}
	}
}

}

#ifdef __cplusplus
extern "C" {
#endif

	typedef cr::gpu_buffer_gl gpu_buffer_t;

	CR_API cr_gpu_buffer cr_gpu_buffer_new( cr_context context, cr_gpu gpu, enum cr_gpu_buffer_type type, cr_uint32 size )
	{
		typedef gpu_buffer_t::cmd_args args_t;
		cr_assert( CR_COMMAND_ARGS_SIZE >= sizeof( args_t ) );
		
		cr_assert( cr::context::singleton );

		gpu_buffer_t* self = new gpu_buffer_t( cr_context_get( context ), ( cr::gpu_gl* )gpu );
		self->size = size;
		self->type = type;

		args_t* args = nullptr;
		cr_command_queue_produce( self->gpu->feeding_queue->cmd_queue, ( cr_command_args* )&args, gpu_buffer_t::create );

		args->self = self;

		return ( cr_gpu_buffer )self;
	}

	CR_API void cr_gpu_buffer_update( cr_gpu_buffer s, cr_uint32 offset, cr_uint32 size, void* data, struct cr_gpu_callback on_complete )
	{
		typedef gpu_buffer_t::cmd_args args_t;
		cr_assert( CR_COMMAND_ARGS_SIZE >= sizeof( args_t ) );

		gpu_buffer_t* self = ( gpu_buffer_t* )s;

		args_t* args = nullptr;
		cr_command_queue_produce( self->gpu->feeding_queue->cmd_queue, ( cr_command_args* )&args, gpu_buffer_t::update );

		args->self = self;
		args->offset = offset;
		args->size = size;
		args->data = cr_mem_alloc( size );
		args->callback = on_complete;

		memcpy( args->data, data, size );
	}

#ifdef __cplusplus
}
#endif
