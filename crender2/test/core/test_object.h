#ifndef TEST_OBJECT_H
#define TEST_OBJECT_H

#include <private/cr_object.h>

struct test_object : cr::object
{
	static int cnt;
	static void dstor(cr::object* self);
};

cr_object test_object_new(cr_context context);

#endif	// TEST_OBJECT_H