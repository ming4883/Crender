#include "cr_gpu.h"

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

	CR_API cr_gpu_callback cr_gpu_callback_null()
	{
		cr_gpu_callback cb;
		memset( &cb, 0, sizeof( cb ) );
		return cb;
	}

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
