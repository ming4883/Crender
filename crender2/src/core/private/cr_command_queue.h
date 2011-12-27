#ifndef CR_COMMAND_QUEUE_PRIVATE_H
#define CR_COMMAND_QUEUE_PRIVATE_H

#include "../cr_command_queue.h"
#include "cr_object.h"

namespace cr
{

struct command_queue : object
{
	static void dstor(object* obj);
};

}	// namespace cr

#endif	// CR_COMMAND_QUEUE_PRIVATE_H
