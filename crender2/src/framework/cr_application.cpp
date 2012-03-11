#include "private/cr_application.h"

namespace cr
{

application* application::inst = nullptr;

application::application()
{
	memset( this, 0, sizeof( application ) );
	event_queue = new event_queue_t;
	gpu = nullptr;
	gpu_desc.api_major = 2;
	gpu_desc.api_minor = 1;
	gpu_desc.msaa_level = 0;
	gpu_desc.vsync = CR_FALSE;

	inst = this;
}

application::~application()
{
	cr_release( gpu );
	delete event_queue;

	inst = nullptr;
}

void application::push_event( cr_uint32 type, const cr_uint8* value )
{
	cr_app_event evt;
	evt.type = type;
	if ( value ) memcpy( evt.value, value, sizeof( evt.value ) );

	event_queue->push( &evt );
}

cr_bool application::pop_event( cr_app_event* evt )
{
	return event_queue->pop( evt );
}

}	// namespace cr

cr_bool cr_app_pop_event( struct cr_app_event* evt )
{
	cr_assert( cr::application::inst );
	return cr::application::inst->pop_event( evt );
}

cr_gpu cr_app_gpu( void )
{
	cr_assert( cr::application::inst );
	return cr::application::inst->gpu;
}
