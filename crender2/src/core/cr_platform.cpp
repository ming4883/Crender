#include "cr_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

CR_API static cr_mem_callback g_callback = {::malloc, ::free};

/*! set custom memory allocation callbacks*/
CR_API void cr_mem_set_callback(cr_mem_callback callback)
{
	g_callback = callback;
	if(nullptr == g_callback.alloc_fn || nullptr == g_callback.free_fn) {
		g_callback.alloc_fn = ::malloc;
		g_callback.free_fn = ::free;
	}
}

/*! allocate memory */
CR_API void* cr_mem_alloc(unsigned int size_in_bytes)
{
	return g_callback.alloc_fn(size_in_bytes);
}

/*! deallocate memory */
CR_API void cr_mem_free(void* ptr)
{
	g_callback.free_fn(ptr);
}

#ifdef __cplusplus
}
#endif