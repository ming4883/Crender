#include "test_object.h"

#include <private/cr_context.h>

int test_object::cnt = 0;

test_object::test_object( cr::context* ctx )
	: object( ctx )
{
	++cnt;
}

test_object::~test_object( void )
{
	--cnt;
}

cr_object test_object_new( cr_context context )
{
	CR_ASSERT( cr::context::singleton );

	return ( cr_object )new test_object( cr_context_get( context ) );
}