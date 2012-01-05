#include <unittest++.h>
#include <cr_command_queue.h>

TEST(cr_command_queue_new_del)
{
	cr_context_initialize();

	cr_command_queue q = cr_command_queue_new(nullptr);

	cr_release(q);

	cr_context_finalize();
}

struct test_cr_command_queue
{
	static int counter;
	static void cmd(cr_command_queue cmd_queue, void* arg)
	{
		++counter;
	}
};

int test_cr_command_queue::counter = 0;

TEST(cr_command_queue_produce_consume)
{
	cr_context_initialize();

	cr_command_queue q = cr_command_queue_new(nullptr);

	test_cr_command_queue::counter = 0;
	cr_command_queue_produce(q, test_cr_command_queue::cmd, nullptr);

	CHECK_EQUAL(0, test_cr_command_queue::counter);

	cr_command_queue_consume(q);
	CHECK_EQUAL(1, test_cr_command_queue::counter);

	// make sure nothing is happened
	cr_command_queue_consume(q);
	CHECK_EQUAL(1, test_cr_command_queue::counter);

	cr_release(q);

	cr_context_finalize();
}
