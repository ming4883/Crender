#include <unittest++.h>
#include <cr_context.h>
#include "test_object.h"

TEST( cr_data_type )
{
	CHECK_EQUAL( 1, sizeof( cr_int8 ) );
	CHECK_EQUAL( 1, sizeof( cr_uint8 ) );

	CHECK_EQUAL( 2, sizeof( cr_int16 ) );
	CHECK_EQUAL( 2, sizeof( cr_uint16 ) );

	CHECK_EQUAL( 4, sizeof( cr_int32 ) );
	CHECK_EQUAL( 4, sizeof( cr_uint32 ) );

	CHECK_EQUAL( sizeof( void* ), sizeof( cr_ptr ) );
	CHECK_EQUAL( sizeof( char* ), sizeof( cr_ptr ) );

	printf( "int=%d\n", sizeof( int ) );
	printf( "long=%d\n", sizeof( long ) );
	printf( "void*=%d\n", sizeof( void* ) );
	printf( "char*=%d\n", sizeof( char* ) );
}

TEST( cr_context_init_final )
{
	cr_context_initialize();
	cr_context_finalize();
}

TEST( cr_context_object_retain_release )
{
	cr_context_initialize();

	cr_object t = test_object_new( nullptr );

	CHECK_EQUAL( 1, test_object::cnt );	// ref-count of t should be 1
	CHECK_EQUAL( 1, cr_ref_count_of( t ) );

	cr_retain( t );	// increase ref-count of t
	CHECK_EQUAL( 2, cr_ref_count_of( t ) );

	cr_release( t );	// decrease ref-count of t
	CHECK_EQUAL( 1, cr_ref_count_of( t ) );

	cr_release( t );	// decrease ref-count of t, t should be deleted
	CHECK_EQUAL( 0, test_object::cnt );

	cr_context_finalize();
}

TEST( cr_context_auto_delete_objects )
{
	cr_context_initialize();

	cr_object t = test_object_new( nullptr );

	CHECK_EQUAL( 1, test_object::cnt );

	cr_context_finalize();

	CHECK_EQUAL( 0, test_object::cnt );
}
