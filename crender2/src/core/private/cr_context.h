#ifndef CR_CONTEXT_PRIVATE_H
#define CR_CONTEXT_PRIVATE_H

#include "../cr_context.h"
#include "cr_object.h"

namespace cr
{

#define cr_context_get(ctx) ((nullptr == ctx) ? cr::context::singleton : (cr::context*)ctx)

struct CR_API context
{
	static context* singleton;

	object* object_list;

	void add_object( object* obj );
	void rmv_object( object* obj );
};

}	// namespace cr

#endif	// CR_CONTEXT_PRIVATE_H
