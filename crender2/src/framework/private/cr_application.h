#ifndef CR_APPLICATION_PRIVATE_H
#define CR_APPLICATION_PRIVATE_H

#include "../cr_application.h"
#include <tinythread/tinythread.h>

namespace cr
{

struct application
{
	application();
	~application();

	static application inst;

	struct event
	{
		cr_app_event evt;
		event* next;
	};

	typedef tthread::mutex mutex_t;
	typedef tthread::lock_guard<mutex_t> lock_guard_t;

	mutex_t* evt_mutex;
	event* evt_list;

	void push_event(cr_uint32 type, const cr_uint8* value);
	cr_bool pop_event(cr_app_event* evt);

};	// application

}	// namespace cr

#endif	// CR_APPLICATION_PRIVATE_H
