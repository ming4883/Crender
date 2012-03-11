#ifndef CR_TS_QUEUE_PRIVATE_H
#define CR_TS_QUEUE_PRIVATE_H

#include "../cr_platform.h"
#include "../uthash/utlist.h"

#include "cr_thread.h"

namespace cr
{

template<typename T>
struct ts_queue
{
	struct item
	{
		T value;
		item* next;
	};

	tt_mutex* mutex;
	item* head;

	ts_queue()
	{
		head = nullptr;
		mutex = new tt_mutex;
	}

	~ts_queue()
	{
		clear();
		delete mutex;
	}

	T* push( T* value )
	{
		item* i = ( item* )cr_mem_alloc( sizeof( item ) );
		memset( i, 0, sizeof( item ) );
		if ( value ) memcpy( &i->value, value, sizeof( T ) );

		{
			tt_lock_guard lock( *mutex );
			LL_APPEND( head, i );
		}

		return &i->value;
	}

	cr_bool pop( T* value )
	{
		if ( nullptr == head ) return CR_FALSE;

		item* i = head;
		memcpy( value, &i->value, sizeof( T ) );

		{
			tt_lock_guard lock( *mutex );
			LL_DELETE( head, head );
		}

		cr_mem_free( i );

		return CR_TRUE;
	}

	void clear()
	{
		tt_lock_guard lock( *mutex );

		item* curr = nullptr, * temp = nullptr;
		LL_FOREACH_SAFE( head, curr, temp )
		{
			LL_DELETE( head, curr );
			cr_mem_free( curr );
		}
	}
};

}	// namespace cr

#endif	// CR_TS_QUEUE_PRIVATE_H
