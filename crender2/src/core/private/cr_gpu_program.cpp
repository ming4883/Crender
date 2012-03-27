#include "cr_gpu_program.h"

namespace cr
{

gpu_program::gpu_program( context* ctx )
	: object( ctx )
	, state( CR_GPU_PROGRAM_NOT_LINKED )
	, log( nullptr )
{
}

gpu_program::~gpu_program( void )
{
	if ( log )
		cr_mem_free( log );
}

}

#ifdef __cplusplus
extern "C" {
#endif

	CR_API cr_gpu_program_state cr_gpu_program_get_state( cr_gpu_program self )
	{
		return ( ( cr::gpu_program* )self )->state;
	}

	CR_API const char* cr_gpu_program_get_log( cr_gpu_program self )
	{
		return ( ( cr::gpu_program* )self )->log;
	}

#ifdef __cplusplus
}
#endif