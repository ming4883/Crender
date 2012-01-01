#include "private/cr_command_queue.h"
#include "private/cr_context.h"

namespace cr
{

void command_queue::dstor(cr::object* obj)
{
}

cr_command_id command_queue::enqueue(cr_command cmd, void* arg)
{
	return 0;
}

void command_queue::execute(void)
{
}

}	// namespace cr

#ifdef __cplusplus
extern "C" {
#endif

CR_API cr_object cr_command_queue_new(void)
{
	CR_ASSERT(cr::context::singleton);

	cr::command_queue* self = cr::context::singleton->new_object<cr::command_queue>();
	self->dstor_func = &cr::command_queue::dstor;

	return (cr_object)self;
}

CR_API cr_command_id cr_command_queue_enqueue(cr_object self, cr_command cmd, void* arg)
{
	if(nullptr == self) return 0;
	if(nullptr == cmd) return 0;

	return ((cr::command_queue*)self)->enqueue(cmd, arg);
}

CR_API void cr_command_queue_execute(cr_object self)
{
	if(nullptr == self) return;
	((cr::command_queue*)self)->execute();
}

#ifdef __cplusplus
}
#endif
