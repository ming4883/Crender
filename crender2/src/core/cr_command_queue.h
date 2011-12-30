#ifndef CR_COMMAND_QUEUE_H
#define CR_COMMAND_QUEUE_H

#include "cr_context.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! a command to be executed */
typedef void (*cr_command) (void* state);

typedef int cr_command_id;

/*! Create and initialize a cr_command_queue.
	cr_command_queue store a queue of cr_command to be executed.
*/
CR_API cr_object cr_command_queue_new(void);

/*! Enqueue a command to a cr_command_queue.
*/
CR_API cr_command_id cr_command_queue_enqueue(cr_object self, cr_command cmd, void* arg);

/*! Execute commands inside the cr_command_queue.
*/
CR_API void cr_command_queue_execute(cr_object self);


#ifdef __cplusplus
}
#endif


#endif	// CR_COMMAND_QUEUE_H
