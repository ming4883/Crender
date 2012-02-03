#ifndef CR_COMMAND_QUEUE_PRIVATE_H
#define CR_COMMAND_QUEUE_PRIVATE_H

#include "../cr_command_queue.h"
#include "cr_object.h"
#include "cr_ts_queue.h"

namespace cr
{

struct command_queue : object
{
	struct command
	{
		cr_command_id id;
		cr_command cmd;
		void* arg;
	};

	typedef ts_queue<command> queue_t;
	queue_t* queue;

	cr_command_id produce_counter;
	cr_command_id consume_counter;

	static void _dstor( object* obj );

	cr_command_id produce( cr_command cmd, void* arg );
	cr_command_id consume( void );
};

}	// namespace cr

#endif	// CR_COMMAND_QUEUE_PRIVATE_H
