#include <unittest++.h>
#include <cr_command_queue.h>

TEST(cr_command_queue_new_del)
{
	cr_context_initialize();

	cr_command_queue q = cr_command_queue_new();

	cr_release(q);

	cr_context_finalize();
}