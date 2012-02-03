#include <unittest++.h>
#include <private/cr_buffer_pool.h>
#include <cr_thread.h>

TEST( cr_buffer_pool_new_del )
{
	cr::buffer_pool* pool = new cr::buffer_pool;

	cr::buffer_pool::buf_t* buf = pool->acquire( 32 );
	pool->release(buf);

	delete pool;
}

TEST( cr_buffer_pool_acquire_release )
{
	cr::buffer_pool* pool = new cr::buffer_pool;

	cr::buffer_pool::buf_t* a = pool->acquire( 32 );
	cr::buffer_pool::buf_t* b = pool->acquire( 32 );
	
	CHECK( a != b );
	CHECK_EQUAL( 64, pool->allocated );

	pool->release( a );
	pool->release( b );

	// make sure the buffers are still alive
	CHECK_EQUAL( 64, pool->allocated );

	// a2 & b2 should be re-used
	cr::buffer_pool::buf_t* a2 = pool->acquire( 32 );
	cr::buffer_pool::buf_t* b2 = pool->acquire( 32 );

	CHECK_EQUAL( a, a2 );
	CHECK_EQUAL( b, b2 );

	CHECK( a2 != b2 );
	CHECK_EQUAL( 64, pool->allocated );

	pool->release( a2 );
	pool->release( b2 );

	delete pool;
}

TEST( cr_buffer_pool_housekeep )
{
	cr::buffer_pool* pool = new cr::buffer_pool;

	cr::buffer_pool::buf_t* a = pool->acquire( 32 );
	cr::buffer_pool::buf_t* b = pool->acquire( 32 );
	
	CHECK( a != b );
	CHECK_EQUAL( 64, pool->allocated );

	pool->release( a );

	for ( int i = 0; i < cr::buffer_pool::INITIAL_TTL; ++i )
		pool->housekeep();

	CHECK_EQUAL( 32, pool->allocated );

	pool->release( b );

	delete pool;
}
