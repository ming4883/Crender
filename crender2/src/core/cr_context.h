#ifndef CR_CONTEXT_H
#define CR_CONTEXT_H

#include "cr_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! cr_context is the core of crender.
	It manages the life cycles all associated cr_object.
*/
typedef unsigned long cr_context;

/*! Initialize the global shared cr_context.
*/
CR_API void cr_context_initialize(void);

/*! Finalize the global shared cr_context and free all associated cr_object */
CR_API void cr_context_finalize(void);

/*! cr_object is the handle to an object in crender.
	All objects in crender is reference counted,
	to retain an object (i.e. increase ref-count) invoke cr_retain.
	to release an object (i.e. decrease ref-count) invoke cr_release.
*/
typedef unsigned long cr_object;

CR_API void cr_retain(cr_object obj);

CR_API void cr_release(cr_object obj);

CR_API unsigned long cr_ref_count_of(cr_object obj);

#ifdef __cplusplus
}
#endif


#endif	// CR_CONTEXT_H
