#ifndef CR_THREAD_H
#define CR_THREAD_H

#include "cr_context.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef cr_object cr_thread;

CR_API cr_thread cr_thread_new(cr_context context, void (*func)(void *), void * arg);

CR_API void cr_thread_join(cr_thread self);

CR_API void cr_thread_sleep(cr_uint32 milliseconds);

#ifdef __cplusplus
}
#endif

#endif	// CR_THREAD_H