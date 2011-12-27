#ifndef CR_COMMAND_QUEUE_H
#define CR_COMMAND_QUEUE_H

#include "cr_context.h"

#ifdef __cplusplus
extern "C" {
#endif

/*! a command to be executed */
typedef void (*cr_command) (void* userdata);

/*! Create and initialize a cr_command_queue.
	cr_command_queue store a queue of cr_command to be executed.
*/
CR_API cr_object cr_command_queue_new(void);


#ifdef __cplusplus
}
#endif


#endif	// CR_COMMAND_QUEUE_H
