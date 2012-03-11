#ifndef TEST_OBJECT_H
#define TEST_OBJECT_H

#include <private/cr_object.h>

struct test_object : cr::object
{
	CR_OVERRIDE_NEW_DELETE();

	static int cnt;

	test_object( cr::context* ctx );
	~test_object( void );
};

cr_object test_object_new( cr_context context );

#endif	// TEST_OBJECT_H
