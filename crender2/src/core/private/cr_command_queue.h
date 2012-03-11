#ifndef CR_COMMAND_QUEUE_PRIVATE_H
#define CR_COMMAND_QUEUE_PRIVATE_H

#include "../cr_command_queue.h"
#include "cr_object.h"
#include "cr_ts_queue.h"

namespace cr
{

struct command_queue : object
{
	CR_OVERRIDE_NEW_DELETE();

	struct command
	{
		char data[CR_COMMAND_ARGS_SIZE + 8];

		cr_command_id& id() { return *( ( cr_command_id* )&data[0] ); }
		cr_command& cmd() { return *( ( cr_command* )&data[4] ); }
		cr_command_args& args() { return *( ( cr_command_args* )&data[8] ); }
	};

	typedef ts_queue<command> queue_t;
	queue_t* queue;

	cr_command_id produce_counter;
	cr_command_id consume_counter;

	command_queue( context* ctx );
	~command_queue( void );

	cr_command_id produce( char** out_args, cr_command cmd );
	cr_command_id consume( void );
};

}	// namespace cr

#endif	// CR_COMMAND_QUEUE_PRIVATE_H
