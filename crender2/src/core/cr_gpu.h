#ifndef CR_GPU_H
#define CR_GPU_H

#include "cr_context.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! cr_gpu repersents a physical gpu device, it is the core of all
	gpu related objects.
*/
typedef cr_object cr_gpu;

struct cr_gpu_desc
{
	cr_uint32 api_major;
	cr_uint32 api_minor;
	cr_uint32 msaa_level;
	cr_bool vsync;
};

/*! Create and initialize a cr_gpu object on the.specific window
*/
CR_API cr_gpu cr_gpu_new(cr_context context, void** window, struct cr_gpu_desc* desc);

/*! Flush and execute the stored command in the cr_gpu object
*/
CR_API void cr_gpu_flush(cr_gpu self);

/*! swap the back buffer contents to the front buffer
*/
CR_API void cr_gpu_swap_buffers(cr_gpu self);

/*! Apply the viewport to the cr_gpu.
*/
CR_API void cr_gpu_set_viewport(cr_gpu self, float x, float y, float w, float h, float zmin, float zmax);

/*! clear the current framebuffer
*/
CR_API void cr_gpu_clear_color(cr_gpu self, float r, float g, float b, float a);

/*! clear the current depthbuffer
*/
CR_API void cr_gpu_clear_depth(cr_gpu self, float z);

#ifdef __cplusplus
}
#endif


#endif	// CR_GPU_H
