#ifndef CR_OBJECT_PRIVATE_H
#define CR_OBJECT_PRIVATE_H

#include "../cr_context.h"

namespace cr
{

struct context;

#define CR_OVERRIDE_NEW_DELETE() \
void* operator new ( unsigned int count )\
{\
	return cr_mem_alloc( count );\
} \
void operator delete( void * ptr )\
{\
	cr_mem_free( ptr );\
}

struct object
{
	CR_OVERRIDE_NEW_DELETE();

	object *next, *prev;
	context* _context;

	cr_uint32 ref_cnt;

	object( context* ctx );
	virtual ~object( void );
};

}	// namespace cr

#endif	// CR_OBJECT_PRIVATE_H
