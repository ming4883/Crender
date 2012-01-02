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

	tthread::mutex* mutex;
	cr_command_id id_counter;
	item* head;

	static void dstor(object* obj);

	cr_command_id produce(cr_command cmd, void* arg);
	void consume(void);
};

}	// namespace cr

#endif	// CR_COMMAND_QUEUE_PRIVATE_H
