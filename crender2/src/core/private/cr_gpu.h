#ifndef CR_GPU_PRIVATE_H
#define CR_GPU_PRIVATE_H

#include "../cr_gpu.h"
#include "cr_object.h"
#include "cr_ts_queue.h"

namespace cr
{

struct gpu : object
{
	CR_OVERRIDE_NEW_DELETE();

	cr_uint32 xres, yres;

	gpu( context* ctx );
	virtual ~gpu( void );
};

}	// namespace cr

#endif	// CR_GPU_PRIVATE_H
