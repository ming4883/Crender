#ifndef CR_APPLICATION_PRIVATE_H
#define CR_APPLICATION_PRIVATE_H

#include "../cr_application.h"
#include <private/cr_ts_queue.h>

namespace cr
{

struct application
{
	static application* inst;

	typedef ts_queue<cr_app_event> event_queue_t;
	event_queue_t* event_queue;

	cr_gpu gpu;
	cr_gpu_desc gpu_desc;

	application();
	~application();

	void push_event( cr_uint32 type, const cr_uint8* value );
	cr_bool pop_event( cr_app_event* evt );

};	// application

}	// namespace cr

#endif	// CR_APPLICATION_PRIVATE_H
