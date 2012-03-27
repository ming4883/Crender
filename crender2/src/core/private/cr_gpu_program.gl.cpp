#include "cr_gpu_program.gl.h"
#include "cr_gpu_shader.gl.h"
#include "cr_context.h"

namespace cr
{

gpu_program_gl::gpu_program_gl( context* ctx, gpu_gl* g )
	: gpu_program( ctx )
	, gpu( g )
	, gl_name( 0 )
{
	cr_retain( ( cr_object )gpu );
}

gpu_program_gl::~gpu_program_gl( void )
{
	if ( gl_name )
		glDeleteProgram( gl_name );

	cr_release( ( cr_object )gpu );
}

void gpu_program_gl::create( cr_command_queue cmd_queue, cr_command_args a )
{
	cmd_args* args = ( cmd_args* )a;

	gpu_program_gl* self =  args->self;
	self->gl_name = glCreateProgram();

	cr_check_gl_err();

	for ( cr_uint32 i = 0; i < args->shader_count; ++i )
	{
		if ( nullptr != args->shaders[i] )
		{
			glAttachShader( self->gl_name, ( ( gpu_shader_gl* )args->shaders[i] )->gl_name );
			cr_release( args->shaders[i] );
		}
	}

	cr_check_gl_err();

	glLinkProgram( self->gl_name );

	GLint status;
	glGetProgramiv( self->gl_name, GL_LINK_STATUS, &status);

	if ( GL_FALSE == status )
	{
		GLint len;
		glGetProgramiv( self->gl_name, GL_INFO_LOG_LENGTH, &len);
		if(len > 0)
		{
			self->log = (char*)cr_mem_alloc( len );
			glGetProgramInfoLog( self->gl_name, len, nullptr, self->log );
		}

		self->state = CR_GPU_PROGRAM_LINK_FAILED;
	}
	else
	{
		self->state = CR_GPU_PROGRAM_LINK_SUCCEED;
	}

	cr_check_gl_err();

	cr_mem_free( args->shaders );
}

}

#ifdef __cplusplus
extern "C" {
#endif

	typedef cr::gpu_program_gl gpu_program_t;

	CR_API cr_gpu_program cr_gpu_program_new( cr_context context, cr_gpu gpu, const cr_gpu_shader* shaders, cr_uint32 shader_count, struct cr_gpu_callback on_complete )
	{
		typedef gpu_program_t::cmd_args args_t;
		cr_assert( CR_COMMAND_ARGS_SIZE >= sizeof( args_t ) );
		
		cr_assert( cr::context::singleton );

		gpu_program_t* self = new gpu_program_t( cr_context_get( context ), ( cr::gpu_gl* )gpu );
		
		args_t* args = nullptr;
		cr_command_queue_produce( self->gpu->feeding_queue->cmd_queue, ( cr_command_args* )&args, gpu_program_t::create );

		args->self = self;
		args->callback = on_complete;

		args->shader_count = shader_count;
		args->shaders = ( cr_gpu_shader* )cr_mem_alloc( sizeof( cr_gpu_shader ) * shader_count );

		for ( cr_uint32 i = 0; i < shader_count; ++i )
		{
			cr_retain( shaders[i] );
			args->shaders[i] = shaders[i];
		}

		return ( cr_gpu_program )self;
	}

#ifdef __cplusplus
}
#endif
