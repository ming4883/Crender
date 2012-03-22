#include "cr_gpu_shader.h"

namespace cr
{

gpu_shader::gpu_shader( context* ctx )
	: object( ctx )
	, type( ( cr_gpu_shader_type )0 )
	, state( CR_GPU_SHADER_NOT_COMPILED )
	, log( nullptr )
{
}

gpu_shader::~gpu_shader( void )
{
	if ( log )
		cr_mem_free( log );
}

}

#ifdef __cplusplus
extern "C" {
#endif

	CR_API cr_gpu_shader_state cr_gpu_shader_get_state( cr_gpu_shader self )
	{
		return ( ( cr::gpu_shader* )self )->state;
	}

	CR_API const char* cr_gpu_shader_get_log( cr_gpu_shader self )
	{
		return ( ( cr::gpu_shader* )self )->log;
	}

#ifdef __cplusplus
}
#endif