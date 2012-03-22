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

	/*! a command to be executed */
	typedef void ( *cr_gpu_callback_func ) ( cr_object object, int result, void* args );

	struct cr_gpu_callback
	{
		cr_gpu_callback_func func;
		void* args;
	};

	CR_API cr_gpu_callback cr_gpu_callback_null();

	/*! create and initialize a cr_gpu object on the.specific window
	*/
	CR_API cr_gpu cr_gpu_new( cr_context context, void** window, struct cr_gpu_desc* desc );

	/*! retrive the screen resolution
	*/
	CR_API void cr_gpu_screen_resolution( cr_gpu self, cr_uint32* xres, cr_uint32* yres );

	/*! flush and execute the stored command in the cr_gpu object
	*/
	CR_API void cr_gpu_flush( cr_gpu self );

	/*! inform the gpu to begin executing commands
	*/
	CR_API cr_bool cr_gpu_begin( cr_gpu self );

	/*! inform the gpu to end executing commands
	*/
	CR_API void cr_gpu_end( cr_gpu self );

	/*! swap the back buffer contents to the front buffer
	*/
	CR_API void cr_gpu_swap_buffers( cr_gpu self );

	/*! apply the viewport to the cr_gpu.
	*/
	CR_API void cr_gpu_set_viewport( cr_gpu self, float x, float y, float w, float h, float zmin, float zmax );

	/*! clear the current framebuffer
	*/
	CR_API void cr_gpu_clear_color( cr_gpu self, float r, float g, float b, float a );

	/*! clear the current depthbuffer
	*/
	CR_API void cr_gpu_clear_depth( cr_gpu self, float z );

#ifdef __cplusplus
}
#endif


#endif	// CR_GPU_H
