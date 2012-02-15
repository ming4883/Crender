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
	};

	buf_t* used_list;	//!< a list of used buffers
	buf_t* free_list;	//!< a list of free buffers, to be deleted or reused

	cr_uint32 allocated;
	cr_uint32 acquired;
	cr_uint32 released;

	buffer_pool( void );
	~buffer_pool( void );

	void* acquire( cr_uint32 size );
	void release( void* buf );

	template< typename T > T* acquire( void )
	{
		return ( T* )acquire( sizeof( T ) );
	}

	void housekeep( void );
	void clear( void );
};

}	// namespace cr


#endif	// CR_BUFFER_POOL_PRIVATE_H