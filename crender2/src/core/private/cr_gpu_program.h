#ifndef CR_GPU_PROGRAM_PRIVATE_H
#define CR_GPU_PROGRAM_PRIVATE_H

#include "../cr_gpu_program.h"
#include "cr_object.h"

namespace cr
{

struct gpu_program : object
{
	CR_OVERRIDE_NEW_DELETE();

	cr_gpu_program_state state;
	char* log;

	gpu_program( context* ctx );
	virtual ~gpu_program( void );
};

}	// namespace cr

#endif	//CR_GPU_PROGRAM_PRIVATE_H
