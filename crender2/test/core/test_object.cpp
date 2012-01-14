#include "test_object.h"

#include <private/cr_context.h>

int test_object::cnt = 0;

void test_object::_dstor(cr::object* self)
{
	--cnt;
}

cr_object test_object_new(cr_context context)
{
	CR_ASSERT(cr::context::singleton);

	test_object* self = cr_context_get(context)->new_object<test_object>();
	
	++test_object::cnt;

	return (cr_object)self;
}