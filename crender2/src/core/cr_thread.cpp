#include "private/cr_thread.h"
#include "private/cr_context.h"

namespace cr
{

thread::thread( context* ctx, void ( *func )( void * ), void * arg )
	: object( ctx )
{
	thread_obj = new tt_thread( func, arg );
}

thread::~thread( void )
{
	delete thread_obj;
}

}	// namespace cr

CR_API cr_thread cr_thread_new( cr_context context, void ( *func )( void * ), void * arg )
{
	CR_ASSERT( cr::context::singleton );

	return ( cr_thread )new cr::thread( cr_context_get( context ), func, arg );
}

CR_API void cr_thread_join( cr_thread self )
{
	if ( nullptr == self ) return;

	return ( ( cr::thread* )self )->thread_obj->join();
}

CR_API void cr_thread_sleep( cr_uint32 milliseconds )
{
	tthread::this_thread::sleep_for( tthread::chrono::milliseconds( milliseconds ) );
}
