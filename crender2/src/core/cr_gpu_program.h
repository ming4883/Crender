#ifndef CR_GPU_PROGRAM_H
#define CR_GPU_PROGRAM_H

#include "cr_context.h"
#include "cr_gpu.h"
#include "cr_gpu_shader.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef enum cr_gpu_program_state
	{
		CR_GPU_PROGRAM_NOT_LINKED,		//!< not linked
		CR_GPU_PROGRAM_LINK_SUCCEED,	//!< link succeed
		CR_GPU_PROGRAM_LINK_FAILED,		//!< link failed
	};

	/*! cr_gpu_program repersents a shader used by gpu device.
	*/
	typedef cr_object cr_gpu_program;

	/*! create and initialize a cr_gpu_program object.
	*/
	CR_API cr_gpu_program cr_gpu_program_new( cr_context context, cr_gpu gpu, const cr_gpu_shader* shaders, cr_uint32 shader_count, struct cr_gpu_callback on_complete );

	/*! retrieve the compile state of a cr_gpu_program object.
	*/
	CR_API enum cr_gpu_program_state cr_gpu_program_get_state( cr_gpu_program self );

	/*! retrieve the compile log of a cr_gpu_program object.
	*/
	CR_API const char* cr_gpu_program_get_log( cr_gpu_program self );

#ifdef __cplusplus
}
#endif


#endif	// CR_GPU_PROGRAM_H
