#ifndef CR_GPU_BUFFER_H
#define CR_GPU_BUFFER_H

#include "cr_context.h"
#include "cr_gpu.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef enum cr_gpu_shader_type
	{
		CR_GPU_SHADER_VERTEX,	//!< vertex shader
		CR_GPU_SHADER_FRAGMENT,	//!< fragment shader
	};

	typedef enum cr_gpu_shader_state
	{
		CR_GPU_SHADER_NOT_COMPILED,		//!< not compiled
		CR_GPU_SHADER_COMPILE_SUCCEED,	//!< compile succeed
		CR_GPU_SHADER_COMPILE_FAILED,	//!< compile failed
	};

	/*! cr_gpu_shader repersents a shader used by gpu device.
	*/
	typedef cr_object cr_gpu_shader;

	/*! create and initialize a cr_gpu_shader object.
	*/
	CR_API cr_gpu_shader cr_gpu_shader_new( cr_context context, cr_gpu gpu, enum cr_gpu_shader_type type, const char* source, struct cr_gpu_callback oncomplete );

	/*! retrieve the compile state of a cr_gpu_shader object.
	*/
	CR_API enum cr_gpu_shader_state cr_gpu_shader_get_state( cr_gpu_shader self );

	/*! retrieve the compile log of a cr_gpu_shader object.
	*/
	CR_API const char* cr_gpu_shader_get_log( cr_gpu_shader self );

#ifdef __cplusplus
}
#endif


#endif	// CR_GPU_H
