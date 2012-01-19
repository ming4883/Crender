#include "private/cr_gpu.h"
#include "private/cr_context.h"

namespace cr
{

void gpu::_dstor(cr::object* obj)
{
	gpu* self = (gpu*)obj;
}

}	// namespace cr

#ifdef __cplusplus
extern "C" {
#endif

CR_API cr_gpu cr_gpu_new(cr_context context, void** window)
{
	CR_ASSERT(cr::context::singleton);

	cr::gpu* self = cr_context_get(context)->new_object<cr::gpu>();

	return (cr_gpu)self;
}

CR_API void cr_gpu_swap_buffers(cr_gpu self)
{
}

#ifdef __cplusplus
}
#endif
