#ifndef CR_TS_QUEUE_PRIVATE_H
#define CR_TS_QUEUE_PRIVATE_H

#include "../cr_platform.h"
#include "../tinythread/tinythread.h"
#include "../uthash/utlist.h"

namespace cr
{

template<typename T>
struct ts_queue
{
	struct item_t
	{
		T value;
		item_t* next;
	};

	typedef tthread::mutex mutex_t;
	typedef tthread::lock_guard<mutex_t> lock_guard_t;

	mutex_t* mutex;
	item_t* head;

	ts_queue()
	{
		head = nullptr;
		mutex = new mutex_t;
	}
	~ts_queue()
	{
		clear();
		delete mutex;
	}

	void push( T* value )
	{
		item_t* i = ( item_t* )cr_mem_alloc( sizeof( item_t ) );
		memset( i, 0, sizeof( item_t ) );
		if ( value ) memcpy( &i->value, value, sizeof( T ) );

		{
			lock_guard_t lock( *mutex );
			LL_APPEND( head, i );
		}
	}

	cr_bool pop( T* value )
	{
		if ( nullptr == head ) return CR_FALSE;

		item_t* i = head;
		memcpy( value, &i->value, sizeof( T ) );

		{
			lock_guard_t lock( *mutex );
			LL_DELETE( head, head );
		}

		cr_mem_free( i );

		return CR_TRUE;
	}

	void clear()
	{
		lock_guard_t lock( *mutex );

		item_t* curr = nullptr, * temp = nullptr;
		LL_FOREACH_SAFE( head, curr, temp )
		{
			LL_DELETE( head, curr );
			cr_mem_free( curr );
		}
	}
};

}	// namespace cr

#endif	// CR_TS_QUEUE_PRIVATE_H
