#include "cr_buffer_pool.h"

namespace cr
{


buffer_pool::buffer_pool( void )
{
	used_list = free_list = nullptr;
	mutex = new mutex_t;
	allocated = 0;
	acquired = 0;
	released = 0;
}

buffer_pool::~buffer_pool( void )
{
	clear();
	delete mutex;
}

void* buffer_pool::acquire( cr_uint32 size )
{
	lock_guard_t lock( *mutex );

	buf_t* bestfit = nullptr;

	// search for a bestfit buffer in free list
	buf_t* curr = nullptr;
	LL_FOREACH( free_list, curr )
	{
		if ( curr->size >= size )
		{
			if ( nullptr == bestfit || curr->size < bestfit->size )
				bestfit = curr;
		}
	}

	if ( bestfit )
	{
		LL_DELETE( free_list, bestfit );
	}
	else
	{
		//todo: http://jongampark.wordpress.com/2008/06/12/implementation-of-aligned-memory-alloc/
		bestfit = ( buf_t* )cr_mem_alloc( sizeof( buf_t ) + size );
		bestfit->size = size;
		bestfit->ptr = ( ( cr_uint8* )bestfit ) + sizeof( buf_t );

		allocated += size;
	}

	bestfit->ttl = INITIAL_TTL;
	LL_APPEND( used_list, bestfit );
	++acquired;

	return bestfit->ptr;
}

void buffer_pool::release( void* ptr )
{
	buf_t* buf = ( buf_t* )( ( cr_uint8* )ptr - sizeof( buf_t ) );
	lock_guard_t lock( *mutex );
	LL_DELETE( used_list, buf );
	LL_APPEND( free_list, buf );
	++released;
}

void buffer_pool::housekeep( void )
{
	buf_t* curr = nullptr;
	buf_t* tmp = nullptr;

	lock_guard_t lock( *mutex );

	LL_FOREACH_SAFE( free_list, curr, tmp )
	{
		if ( --curr->ttl == 0 )
		{
			allocated -= curr->size;
			LL_DELETE( free_list, curr );
			cr_mem_free( curr );
		}
	}
}

void buffer_pool::clear( void )
{
	buf_t* curr = nullptr;
	buf_t* tmp = nullptr;

	lock_guard_t lock( *mutex );

	LL_FOREACH_SAFE( free_list, curr, tmp )
	{
		LL_DELETE( free_list, curr );
		cr_mem_free( curr );
	}

	if ( used_list )
	{
		cr_dbg_str( "buffer_pool 0x%08x is cleared while some buffers are still in used\n", ( int )this );

		LL_FOREACH_SAFE( used_list, curr, tmp )
		{
			LL_DELETE( used_list, curr );
			cr_mem_free( curr );
		}
	}

	allocated = 0;
	acquired = 0;
	released = 0;
}


} // namespace cr
