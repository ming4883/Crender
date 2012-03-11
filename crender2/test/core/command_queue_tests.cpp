#include <unittest++.h>
#include <cr_command_queue.h>
#include <cr_thread.h>

struct test_command_queue
{
	static int counter;
	static void cmd( cr_command_queue cmd_queue, cr_command_args arg )
	{
		++counter;
	}
};

int test_command_queue::counter = 0;

TEST( cr_command_queue_new_del )
{
	cr_context_initialize();

	cr_command_queue q = cr_command_queue_new( nullptr );

	test_command_queue::counter = 0;
	cr_command_queue_produce( q, nullptr, test_command_queue::cmd );
	cr_command_queue_produce( q, nullptr, test_command_queue::cmd );
	CHECK_EQUAL( 0, test_command_queue::counter );

	cr_release( q );

	cr_context_finalize();
}

TEST( cr_command_queue_produce_consume )
{
	cr_context_initialize();

	cr_command_queue q = cr_command_queue_new( nullptr );

	test_command_queue::counter = 0;
	cr_command_queue_produce( q, nullptr, test_command_queue::cmd );

	CHECK_EQUAL( 0, test_command_queue::counter );

	cr_command_queue_consume( q );
	CHECK_EQUAL( 1, test_command_queue::counter );

	// make sure test_command_queue::counter is not being changed
	cr_command_queue_consume( q );
	CHECK_EQUAL( 1, test_command_queue::counter );

	cr_release( q );

	cr_context_finalize();
}

TEST( cr_command_queue_is_consumed )
{
	cr_context_initialize();

	cr_command_queue q = cr_command_queue_new( nullptr );

	test_command_queue::counter = 0;
	cr_command_id id0 = cr_command_queue_produce( q, nullptr, test_command_queue::cmd );
	cr_command_id id1 = cr_command_queue_produce( q, nullptr, test_command_queue::cmd );
	cr_command_id id2 = cr_command_queue_produce( q, nullptr, test_command_queue::cmd );

	CHECK_EQUAL( 0, test_command_queue::counter );

	while ( !cr_command_queue_is_consumed( q, id0 ) )
	{
		cr_command_queue_consume( q );
	}

	CHECK_EQUAL( 1, test_command_queue::counter );

	while ( !cr_command_queue_is_consumed( q, id1 ) )
	{
		cr_command_queue_consume( q );
	}

	CHECK_EQUAL( 2, test_command_queue::counter );

	while ( !cr_command_queue_is_consumed( q, id2 ) )
	{
		cr_command_queue_consume( q );
	}

	CHECK_EQUAL( 3, test_command_queue::counter );

	cr_release( q );

	cr_context_finalize();
}

struct test_command_queue_multi_thread_producer_consumer
{
	typedef test_command_queue_multi_thread_producer_consumer state;

	enum {CMD_COUNT = 20};

	cr_command_queue q;

	static void producer( void* arg )
	{
		state& s = *( state* )arg;

		test_command_queue::counter = 0;

		for ( int i = 0; i < CMD_COUNT; ++i )
		{
			cr_command_id id = cr_command_queue_produce( s.q, nullptr, test_command_queue::cmd );
#ifdef _DEBUG
			printf( "produced %d\n", id );
#endif
			cr_thread_sleep( 1 );
		}
	}

	static void consumer( void* arg )
	{
		state& s = *( state* )arg;

		while ( test_command_queue::counter < CMD_COUNT )
		{
			cr_command_id id = cr_command_queue_consume( s.q );
#ifdef _DEBUG
			if ( id != 0 ) printf( "consumed %d\n", id );
#endif
		}
	}
};

TEST( cr_command_queue_multi_thread_producer_consumer )
{
	typedef test_command_queue_multi_thread_producer_consumer state;

	cr_context_initialize();

	state s;
	s.q = cr_command_queue_new( nullptr );
	cr_thread p_thread = cr_thread_new( nullptr, state::producer, &s );
	cr_thread c_thread = cr_thread_new( nullptr, state::consumer, &s );

	cr_thread_join( p_thread );
	cr_thread_join( c_thread );

	cr_context_finalize();
}