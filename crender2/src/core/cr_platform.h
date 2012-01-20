#ifndef CR_PLATFORM_H
#define CR_PLATFORM_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define CR_API

#ifdef __cplusplus
extern "C" {
#endif

// nullptr
#ifndef nullptr
#define nullptr 0
#endif

// debug flag
#if defined(DEBUG) || defined(_DEBUG)
#	define CR_DEBUG 1
#else
#	define CR_DEBUG 0
#endif

// cr_bool
typedef unsigned char cr_bool;

#define CR_TRUE 1
#define CR_FALSE 0

// integer types
typedef char cr_int8;
typedef short cr_int16;
typedef int cr_int32;

typedef unsigned char cr_uint8;
typedef unsigned short cr_uint16;
typedef unsigned int cr_uint32;

typedef unsigned long cr_ptr;

typedef void*(* cr_alloc_func) (size_t size_in_bytes);
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

/*! debug printf */
CR_API void cr_printf(const char* str, ...);

#define cr_dbg_str(str, ...) { if(CR_DEBUG) cr_printf(str, __VA_ARGS__); }

#ifdef __cplusplus
}
#endif

#endif // CR_PLATFORM_H
