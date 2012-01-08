#include <unittest++.h>
#include <cr_command_queue.h>
#include <tinythread/tinythread.h>

struct test_command_queue
{
	static int counter;
	static void cmd(cr_command_queue cmd_queue, void* arg)
	{
		++counter;
	}
};

int test_command_queue::counter = 0;

TEST(cr_command_queue_new_del)
{
	cr_context_initialize();

	cr_command_queue q = cr_command_queue_new(nullptr);

	test_command_queue::counter = 0;
	cr_command_queue_produce(q, test_command_queue::cmd, nullptr);
	cr_command_queue_produce(q, test_command_queue::cmd, nullptr);
	CHECK_EQUAL(0, test_command_queue::counter);

	cr_release(q);

	cr_context_finalize();
}

TEST(cr_command_queue_produce_consume)
{
	cr_context_initialize();

	cr_command_queue q = cr_command_queue_new(nullptr);

	test_command_queue::counter = 0;
	cr_command_queue_produce(q, test_command_queue::cmd, nullptr);

	CHECK_EQUAL(0, test_command_queue::counter);

	cr_command_queue_consume(q);
	CHECK_EQUAL(1, test_command_queue::counter);

	// make sure test_command_queue::counter is not being changed
	cr_command_queue_consume(q);
	CHECK_EQUAL(1, test_command_queue::counter);

	cr_release(q);

	cr_context_finalize();
}

TEST(cr_command_queue_is_consumed)
{
	cr_context_initialize();

	cr_command_queue q = cr_command_queue_new(nullptr);

	test_command_queue::counter = 0;
	cr_command_id id0 = cr_command_queue_produce(q, test_command_queue::cmd, nullptr);
	cr_command_id id1 = cr_command_queue_produce(q, test_command_queue::cmd, nullptr);
	cr_command_id id2 = cr_command_queue_produce(q, test_command_queue::cmd, nullptr);

	CHECK_EQUAL(0, test_command_queue::counter);

	while(!cr_command_queue_is_consumed(q, id0)) {
		cr_command_queue_consume(q);
	}

	CHECK_EQUAL(1, test_command_queue::counter);

	while(!cr_command_queue_is_consumed(q, id1)) {
		cr_command_queue_consume(q);
	}

	CHECK_EQUAL(2, test_command_queue::counter);

	while(!cr_command_queue_is_consumed(q, id2)) {
		cr_command_queue_consume(q);
	}

	CHECK_EQUAL(3, test_command_queue::counter);
	
	cr_release(q);

	cr_context_finalize();
}

struct test_command_queue_multi_thread_producer_consumer
{
	typedef test_command_queue_multi_thread_producer_consumer state;

	enum {CMD_COUNT = 20};

	cr_command_queue q;

	static void producer(void* arg)
	{
		state& s = *(state*)arg;

		test_command_queue::counter = 0;

		for(int i=0; i<CMD_COUNT; ++i) {
			cr_command_id id = cr_command_queue_produce(s.q, test_command_queue::cmd, nullptr);
			//printf("produced %d\n", id);
			tthread::this_thread::yield();
		}
	}

	static void consumer(void* arg)
	{
		state& s = *(state*)arg;

		while(test_command_queue::counter < CMD_COUNT) {
			cr_command_id id = cr_command_queue_consume(s.q);
			//if(id != 0) printf("consumed %d\n", id);
		}
	}
};

TEST(cr_command_queue_multi_thread_producer_consumer)
{
	typedef test_command_queue_multi_thread_producer_consumer state;

	state s;

	tthread::thread p_thread(state::producer, &s);
	tthread::thread c_thread(state::consumer, &s);

	cr_context_initialize();

	s.q = cr_command_queue_new(nullptr);

	p_thread.join();
	c_thread.join();

	cr_release(s.q);

	cr_context_finalize();
}