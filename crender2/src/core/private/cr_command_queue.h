#ifndef CR_COMMAND_QUEUE_PRIVATE_H
#define CR_COMMAND_QUEUE_PRIVATE_H

#include "../cr_command_queue.h"
#include "cr_object.h"

#include "../tinythread/tinythread.h"

namespace cr
{

struct command_queue : object
{
	struct item
	{
		cr_command_id id;
		cr_command cmd;
		void* arg;

		item* next;
	};

	typedef tthread::mutex mutex_t;
	typedef tthread::lock_guard<mutex_t> lock_guard_t;

	mutex_t* mutex;
	cr_command_id produce_counter;
	cr_command_id consume_counter;
	item* head;

	static void _dstor(object* obj);

	cr_command_id produce(cr_command cmd, void* arg);
	cr_command_id consume(void);
};

}	// namespace cr

#endif	// CR_COMMAND_QUEUE_PRIVATE_H
