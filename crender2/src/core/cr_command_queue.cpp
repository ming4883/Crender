#include "private/cr_command_queue.h"
#include "private/cr_context.h"

namespace cr
{

void command_queue::dstor(cr::object* obj)
{
}

}	// namespace cr

#ifdef __cplusplus
extern "C" {
#endif

CR_API cr_object cr_command_queue_new(void)
{
	CR_ASSERT(cr::context::singleton);

	cr::command_queue* _self = cr::context::singleton->new_object<cr::command_queue>();
	_self->dstor_func = &cr::command_queue::dstor;

	return (cr_object)_self;
}

#ifdef __cplusplus
}
#endif
