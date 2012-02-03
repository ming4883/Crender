#include "private/cr_thread.h"
#include "private/cr_context.h"

namespace cr
{

void thread::_dstor( cr::object* obj )
{
	thread* self = ( thread* )obj;

	delete self->thread_obj;
}

}	// namespace cr

CR_API cr_thread cr_thread_new( cr_context context, void ( *func )( void * ), void * arg )
{
	CR_ASSERT( cr::context::singleton );

	cr::thread* self = cr_context_get( context )->new_object<cr::thread>();

	self->thread_obj = new cr::thread::thread_t( func, arg );

	return ( cr_thread )self;
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
