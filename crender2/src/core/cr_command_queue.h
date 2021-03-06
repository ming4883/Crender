#ifndef CR_COMMAND_QUEUE_H
#define CR_COMMAND_QUEUE_H

#include "cr_context.h"

#ifdef __cplusplus
extern "C" {
#endif

	/*! cr_command_queue store a queue of cr_command to be executed.
	*/
	typedef cr_object cr_command_queue;

	/*! a fixed size command argument buffer */
	typedef char* cr_command_args;
	#define CR_COMMAND_ARGS_SIZE 72

	/*! a command to be executed */
	typedef void ( *cr_command ) ( cr_command_queue cmd_queue, cr_command_args args );

	typedef int cr_command_id;

	/*! Create and initialize a cr_command_queue.
	*/
	CR_API cr_command_queue cr_command_queue_new( cr_context context );

	/*! Enqueue a command to a cr_command_queue.
	*/
	CR_API cr_command_id cr_command_queue_produce( cr_command_queue self, cr_command_args* out_args, cr_command cmd );

	/*! Dequeue and execute the command in the front of the cr_command_queue.
	*/
	CR_API cr_command_id cr_command_queue_consume( cr_command_queue self );

	/*! Dequeue and execute all commands in the cr_command_queue.
	*/
	CR_API void cr_command_queue_consume_all( cr_command_queue self );

	/*! Check whether a command is consumed or not.
	*/
	CR_API cr_bool cr_command_queue_is_consumed( cr_command_queue self, cr_command_id cmd_id );


#ifdef __cplusplus
}
#endif


#endif	// CR_COMMAND_QUEUE_H
