#ifndef CR_BUFFER_POOL_PRIVATE_H
#define CR_BUFFER_POOL_PRIVATE_H

#include "../cr_platform.h"
#include "../tinythread/tinythread.h"
#include "../uthash/utlist.h"

namespace cr
{

struct buffer_pool
{
	enum { INITIAL_TTL = 64 };

	struct buf_t
	{
		cr_uint32 ttl;
		cr_uint32 size;
		cr_uint8* ptr;
		buf_t* next;

		template<typename T> T& as( void )
		{
			CR_ASSERT( sizeof( T ) <= size );
			return *( ( T* )ptr );
		}
	};

	typedef tthread::mutex mutex_t;
	typedef tthread::lock_guard< mutex_t > lock_guard_t;

	buf_t* used_list;	//!< a list of used buffers
	buf_t* free_list;	//!< a list of free buffers, to be deleted or reused

	mutex_t* mutex;

	cr_uint32 allocated;

	buffer_pool( void );
	~buffer_pool( void );

	buf_t* acquire( cr_uint32 size );
	void release( buf_t* buf );
	
	void housekeep( void );
	void clear( void );
};

}	// namespace cr


#endif	// CR_BUFFER_POOL_PRIVATE_H