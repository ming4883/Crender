#include "cr_gpu_shader.h"

namespace cr
{

gpu_shader::gpu_shader( context* ctx )
	: object( ctx )
	, type( ( cr_gpu_shader_type )0 )
	, size( 0 )
	, sys_mem( nullptr )
{
}

gpu_shader::~gpu_shader( void )
{
}

bool gpu_shader::is_sys_mem( void )
{
	return ( type & CR_GPU_BUFFER_SYSMEM ) > 0;
}

}
