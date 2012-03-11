#ifndef CR_GPU_BUFFER_PRIVATE_H
#define CR_GPU_BUFFER_PRIVATE_H

#include "../cr_gpu_buffer.h"
#include "cr_object.h"

namespace cr
{

struct gpu_buffer : object
{
	CR_OVERRIDE_NEW_DELETE();

	cr_gpu_buffer_type type;
	cr_uint32 size;
	void* sys_mem;

	gpu_buffer( context* ctx );
	virtual ~gpu_buffer( void );

	bool is_sys_mem( void );
};

}	// namespace cr

#endif	// CR_GPU_BUFFER_PRIVATE_H
