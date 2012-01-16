#include "private/cr_application.h"

#include <string.h>
#include <uthash/utlist.h>

namespace cr
{

application application::inst;

application::application()
{
	memset(this, 0, sizeof(application));
	evt_mutex = new mutex_t;
}

application::~application()
{
	delete evt_mutex;
}

void application::push_event(cr_uint32 type, const cr_uint8* value)
{
	event* e = (event*)cr_mem_alloc(sizeof(event));
	memset(e, 0, sizeof(event));
	e->evt.type = type;
	if(nullptr != value)
		memcpy(e->evt.value, value, sizeof(e->evt.value));

	{	lock_guard_t lock(*evt_mutex);
		LL_APPEND(evt_list, e);
	}
}

cr_bool application::pop_event(cr_app_event* evt)
{
	if(nullptr == evt_list) return CR_FALSE;

	event* e = evt_list;

	memcpy(evt, &e->evt, sizeof(cr_app_event));

	{	lock_guard_t lock(*evt_mutex);
		LL_DELETE(evt_list, evt_list);
	}
	
	cr_mem_free(e);

	return CR_TRUE;
}

}	// namespace cr

cr_bool cr_app_pop_event(struct cr_app_event* evt)
{
	return cr::application::inst.pop_event(evt);
}

