#ifndef CR_PLATFORM_H
#define CR_PLATFORM_H

#include <assert.h>

#define CR_API

#ifdef __cplusplus
extern "C" {
#endif

typedef int cr_bool;

#define CR_TRUE 0
#define CR_FALSE 1

#ifndef nullptr
#define nullptr 0
#endif

typedef void*(* cr_alloc_func) (unsigned int size_in_bytes);
typedef void (* cr_free_func) (void* ptr);

#define CR_ASSERT(x) assert(x)

/*! memory allocation / deallocation callback */
struct cr_mem_callback
{
	cr_alloc_func alloc_fn;	/*!< memory allocate function, nullptr = default. */
	cr_free_func free_fn; /*!< memory free function, nullptr = default. */
};

/*! set custom memory allocation callbacks*/
CR_API void cr_mem_set_callback(cr_mem_callback callback);

/*! allocate memory */
CR_API void* cr_mem_alloc(unsigned int size_in_bytes);

/*! deallocate memory */
CR_API void cr_mem_free(void* ptr);

#ifdef __cplusplus
}
#endif

#endif // CR_PLATFORM_H