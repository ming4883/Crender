#include "private/cr_command_queue.h"
#include "private/cr_context.h"
#include "uthash/utlist.h"

namespace cr
{

void command_queue::dstor(cr::object* obj)
{
	command_queue& self = (command_queue&)*obj;
	delete self.mutex;
}

cr_command_id command_queue::produce(cr_command cmd, void* arg)
{
	item* i = (item*)cr_mem_alloc(sizeof(item));
	i->cmd = cmd;
	i->arg = arg;
	i->next = nullptr;

	{	lock_guard_t lock(*mutex);
		i->id = ++produce_counter;
		LL_APPEND(head, i);
	}

	return 0;
}

void command_queue::consume(void)
{
	if(nullptr == head)
		return;

	item* i = head;

	i->cmd((cr_command_queue)this, i->arg);
	
	{	lock_guard_t lock(*mutex);
		LL_DELETE(head, head);
		++consume_counter;
	}

	cr_mem_free(i);
}

}	// namespace cr

#ifdef __cplusplus
extern "C" {
#endif

CR_API cr_command_queue cr_command_queue_new(cr_context context)
{
	CR_ASSERT(cr::context::singleton);

	cr::command_queue* self = cr_context_get(context)->new_object<cr::command_queue>();

	self->dstor_func = &cr::command_queue::dstor;
	self->mutex = new cr::command_queue::mutex_t;
	self->produce_counter = 0;
	self->consume_counter = 0;
	self->head = nullptr;

	return (cr_command_queue)self;
}

CR_API cr_command_id cr_command_queue_produce(cr_command_queue self, cr_command cmd, void* arg)
{
	if(nullptr == self) return 0;
	if(nullptr == cmd) return 0;

	return ((cr::command_queue*)self)->produce(cmd, arg);
}

CR_API void cr_command_queue_consume(cr_command_queue self)
{
	if(nullptr == self) return;
	((cr::command_queue*)self)->consume();
}

CR_API cr_bool cr_command_queue_is_consumed(cr_command_queue self, cr_command_id cmd_id)
{
	if(nullptr == self) return CR_FALSE;
	return ((cr::command_queue*)self)->consume_counter >= cmd_id;
}

#ifdef __cplusplus
}
#endif
