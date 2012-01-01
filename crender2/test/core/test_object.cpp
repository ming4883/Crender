#include "test_object.h"

#include <private/cr_context.h>

int test_object::cnt = 0;

void test_object::dstor(cr::object* self)
{
	--cnt;
}

cr_object test_object_new(void)
{
	CR_ASSERT(cr::context::singleton);

	test_object* self = cr::context::singleton->new_object<test_object>();
	self->dstor_func = &test_object::dstor;

	++test_object::cnt;

	return (cr_object)self;
}