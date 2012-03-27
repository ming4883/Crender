#include "cr_gpu_shader.gl.h"
#include "cr_context.h"

namespace cr
{

gpu_shader_gl::gpu_shader_gl( context* ctx, gpu_gl* g )
	: gpu_shader( ctx )
	, gpu( g )
	, gl_name( 0 )
{
	cr_retain( ( cr_object )gpu );
}

gpu_shader_gl::~gpu_shader_gl( void )
{
	if ( gl_name )
		glDeleteShader( gl_name );

	cr_release( ( cr_object )gpu );
}

void gpu_shader_gl::create( cr_command_queue cmd_queue, cr_command_args a )
{
	static GLenum gl_shader_type[] = {
		GL_VERTEX_SHADER,
		GL_FRAGMENT_SHADER,
	};

	cmd_args* args = ( cmd_args* )a;

	gpu_shader_gl* self =  args->self;
	self->gl_name = glCreateShader( gl_shader_type[ self->type ] );

	cr_check_gl_err();

	glShaderSource( self->gl_name, 1, ( const GLchar** )&args->source, nullptr );
	glCompileShader( self->gl_name );

	GLint status;
	glGetShaderiv( self->gl_name, GL_COMPILE_STATUS, &status);

	if ( GL_FALSE == status )
	{
		GLint len;
		glGetShaderiv( self->gl_name, GL_INFO_LOG_LENGTH, &len);
		if(len > 0)
		{
			self->log = (char*)cr_mem_alloc( len );
			glGetShaderInfoLog( self->gl_name, len, nullptr, self->log );
		}

		self->state = CR_GPU_SHADER_COMPILE_FAILED;
	}
	else
	{
		self->state = CR_GPU_SHADER_COMPILE_SUCCEED;
	}

	cr_check_gl_err();

	cr_mem_free( args->source );
	
}

}

#ifdef __cplusplus
extern "C" {
#endif

	typedef cr::gpu_shader_gl gpu_shader_t;

	CR_API cr_gpu_shader cr_gpu_shader_new( cr_context context, cr_gpu gpu, enum cr_gpu_shader_type type, const char* source, struct cr_gpu_callback on_complete )
	{
		typedef gpu_shader_t::cmd_args args_t;
		cr_assert( CR_COMMAND_ARGS_SIZE >= sizeof( args_t ) );
		
		cr_assert( cr::context::singleton );

		gpu_shader_t* self = new gpu_shader_t( cr_context_get( context ), ( cr::gpu_gl* )gpu );
		self->type = type;

		args_t* args = nullptr;
		cr_command_queue_produce( self->gpu->feeding_queue->cmd_queue, ( cr_command_args* )&args, gpu_shader_t::create );

		args->self = self;
		args->callback = on_complete;

		int len = strlen( source );
		args->source = ( char* )cr_mem_alloc( len );
		memcpy( args->source, source, len );

		return ( cr_gpu_shader )self;
	}

#ifdef __cplusplus
}
#endif
