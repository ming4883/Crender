#include "cr_gpu_buffer.h"

namespace cr
{

gpu_buffer::gpu_buffer( context* ctx )
	: object( ctx )
	, type( ( cr_gpu_buffer_type )0 )
	, size( 0 )
	, sys_mem( nullptr )
{
}

gpu_buffer::~gpu_buffer( void )
{
}

bool gpu_buffer::is_sys_mem( void )
{
	return ( type & CR_GPU_BUFFER_SYSMEM ) > 0;
}

}
