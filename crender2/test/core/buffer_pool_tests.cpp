#include <unittest++.h>
#include <private/cr_buffer_pool.h>
#include <cr_thread.h>

TEST( cr_buffer_pool_new_del )
{
	cr::buffer_pool* pool = new cr::buffer_pool;

	cr::buffer_pool::buf_t* buf = pool->acquire( 32 );
	pool->release( buf );

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

	// a should be deleted after INITIAL_TTL housekeep
	for ( int i = 0; i < cr::buffer_pool::INITIAL_TTL - 1; ++i )
		pool->housekeep();

	CHECK_EQUAL( 64, pool->allocated );

	pool->housekeep();
	CHECK_EQUAL( 32, pool->allocated );

	// ok test against b
	pool->release( b );

	for ( int i = 0; i < cr::buffer_pool::INITIAL_TTL - 1; ++i )
		pool->housekeep();

	CHECK_EQUAL( 32, pool->allocated );

	cr::buffer_pool::buf_t* b2 = pool->acquire( 32 );

	// b should be reused & pool->allocated stay remain 32 bytes
	CHECK_EQUAL( b, b2 );
	CHECK_EQUAL( 32, pool->allocated );

	// release b2 and let pool to clean up b2
	pool->release( b2 );
	for ( int i = 0; i < cr::buffer_pool::INITIAL_TTL; ++i )
		pool->housekeep();

	CHECK_EQUAL( 0, pool->allocated );

	delete pool;
}
