#include "private/cr_application.h"

namespace cr
{

application application::inst;

application::application()
{
	memset( this, 0, sizeof( application ) );
	event_queue = new event_queue_t;
}

application::~application()
{
	delete event_queue;
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
	return cr::application::inst.pop_event( evt );
}

