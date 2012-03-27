#ifndef CR_GPU_BUFFER_H
#define CR_GPU_BUFFER_H

#include "cr_context.h"
#include "cr_gpu.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef enum cr_gpu_buffer_type
	{
		CR_GPU_BUFFER_VERTEX,	//!< vertex buffer
		CR_GPU_BUFFER_INDEX,	//!< 16-bit index buffer
		CR_GPU_BUFFER_INDEX8,	//!< 8-bit index buffer
		CR_GPU_BUFFER_INDEX32,	//!< 32-bit index buffer
		CR_GPU_BUFFER_UNIFORM,	//!< shader uniform buffer
		CR_GPU_BUFFER_MASK		= 0x0000ffff,
		CR_GPU_BUFFER_SYSMEM	= 0x80000000,	//!< if set data will be held in sysMem 
	};

	/*! cr_gpu_buffer repersents a buffer used by gpu device.
	*/
	typedef cr_object cr_gpu_buffer;

	/*! create and initialize a cr_gpu_buffer object.
		\param size size in bytes
	*/
	CR_API cr_gpu_buffer cr_gpu_buffer_new( cr_context context, cr_gpu gpu, enum cr_gpu_buffer_type type, cr_uint32 size );

	/*! update a cr_gpu_buffer object with data stored in the system memory.
		\param offset destination offset in bytes in the cr_gpu_buffer object.
		\param size data size in bytes.
		\param data system memory pointer to the data.
	*/
	CR_API void cr_gpu_buffer_update( cr_gpu_buffer self, cr_uint32 offset, cr_uint32 size, void* data, struct cr_gpu_callback on_complete );

#ifdef __cplusplus
}
#endif


#endif	// CR_GPU_H
