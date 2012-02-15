#include "private/cr_gpu.h"
#include "private/cr_context.h"

namespace cr
{

gpu::gpu( context* ctx )
	: object( ctx )
{
	xres = yres = 0;
}

gpu::~gpu( void )
{
}

}

#ifdef __cplusplus
extern "C" {
#endif

	CR_API void cr_gpu_screen_resolution( cr_gpu s, cr_uint32* xres, cr_uint32* yres )
	{
		cr::gpu* self = ( cr::gpu* )s;

		if ( nullptr != xres )
			*xres = self->xres;

		if ( nullptr != yres )
			*yres = self->yres;
	}

#ifdef __cplusplus
}
#endif
