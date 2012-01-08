#ifndef CR_OBJECT_PRIVATE_H
#define CR_OBJECT_PRIVATE_H

#include "../cr_context.h"

namespace cr
{

struct context;
struct object;
typedef void (*object_dstor_func) (object* obj);

struct object
{
	object_dstor_func dstor_func;
	object *next, *prev;
	context* _context;

	cr_uint32 ref_cnt;

	void cstor(context* c);
};

}	// namespace cr

#endif	// CR_OBJECT_PRIVATE_H
