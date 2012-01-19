#include "private/cr_command_queue.h"
#include "private/cr_context.h"

namespace cr
{

void command_queue::_dstor(cr::object* obj)
{
	command_queue* self = (command_queue*)obj;

	// clean up remaining commands in queue
	delete self->queue;
}

cr_command_id command_queue::produce(cr_command cmd, void* arg)
{
	command i;
	i.cmd = cmd;
	i.arg = arg;
	i.id = ++produce_counter;
	queue->push(&i);

	return produce_counter;
}

cr_command_id command_queue::consume(void)
{
	command i;
	if(!queue->pop(&i))
		return 0;

	++consume_counter;
	i.cmd((cr_command_queue)this, i.arg);
	
	return i.id;
}

}	// namespace cr

#ifdef __cplusplus
extern "C" {
#endif

CR_API cr_command_queue cr_command_queue_new(cr_context context)
{
	CR_ASSERT(cr::context::singleton);

	cr::command_queue* self = cr_context_get(context)->new_object<cr::command_queue>();

	self->queue = new cr::command_queue::queue_t;
	self->produce_counter = 0;
	self->consume_counter = 0;
	
	return (cr_command_queue)self;
}

CR_API cr_command_id cr_command_queue_produce(cr_command_queue self, cr_command cmd, void* arg)
{
	if(nullptr == self) return 0;
	if(nullptr == cmd) return 0;

	return ((cr::command_queue*)self)->produce(cmd, arg);
}

CR_API cr_command_id cr_command_queue_consume(cr_command_queue self)
{
	if(nullptr == self) return 0;
	return ((cr::command_queue*)self)->consume();
}

CR_API cr_bool cr_command_queue_is_consumed(cr_command_queue self, cr_command_id cmd_id)
{
	if(nullptr == self) return CR_FALSE;
	return ((cr::command_queue*)self)->consume_counter >= cmd_id;
}

#ifdef __cplusplus
}
#endif