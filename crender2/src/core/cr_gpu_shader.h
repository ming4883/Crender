#ifndef CR_GPU_BUFFER_H
#define CR_GPU_BUFFER_H

#include "cr_context.h"
#include "cr_gpu.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef enum cr_gpu_shader_type
	{
		CR_GPU_BUFFER_VERTEX,	//!< vertex buffer
		CR_GPU_BUFFER_INDEX,	//!< 16-bit index buffer
		CR_GPU_BUFFER_INDEX8,	//!< 8-bit index buffer
		CR_GPU_BUFFER_INDEX32,	//!< 32-bit index buffer
		CR_GPU_BUFFER_UNIFORM,	//!< shader uniform buffer
		CR_GPU_BUFFER_MASK		= 0x0000ffff,
		CR_GPU_BUFFER_SYSMEM	= 0x80000000,	//!< if set data will be held in sysMem 
	};

	/*! cr_gpu_shader repersents a buffer used by gpu device.
	*/
	typedef cr_object cr_gpu_shader;

	/*! create and initialize a cr_gpu_shader object.
		\param size size in bytes
	*/
	CR_API cr_gpu_shader cr_gpu_shader_new( cr_context context, cr_gpu gpu, enum cr_gpu_shader_type type, cr_uint32 size );

	/*! update a cr_gpu_shader object with data stored in the system memory.
		\param offset destination offset in bytes in the cr_gpu_shader object.
		\param size data size in bytes.
		\param data system memory pointer to the data.
	*/
	CR_API void cr_gpu_shader_update( cr_gpu_shader self, cr_uint32 offset, cr_uint32 size, void* data );

#ifdef __cplusplus
}
#endif


#endif	// CR_GPU_H
