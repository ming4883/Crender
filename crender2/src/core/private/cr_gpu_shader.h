#ifndef CR_GPU_SHADER_PRIVATE_H
#define CR_GPU_SHADER_PRIVATE_H

#include "../cr_gpu_shader.h"
#include "cr_object.h"

namespace cr
{

struct gpu_shader : object
{
	CR_OVERRIDE_NEW_DELETE();

	cr_gpu_shader_type type;
	cr_gpu_shader_state state;
	char* log;

	gpu_shader( context* ctx );
	virtual ~gpu_shader( void );
};

}	// namespace cr

#endif	// CR_GPU_SHADER_PRIVATE_H
