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

/*! Create and initialize a cr_gpu object on the.specific window
*/
CR_API cr_gpu cr_gpu_new(cr_context context, void** window);

/*! swap the back buffer contents to the front buffer
*/
CR_API void cr_gpu_swap_buffers(cr_gpu self);

#ifdef __cplusplus
}
#endif


#endif	// CR_GPU_H
