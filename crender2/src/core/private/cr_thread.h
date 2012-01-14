#ifndef CR_THREAD_PRIVATE_H
#define CR_THREAD_PRIVATE_H

#include "../cr_thread.h"
#include "cr_object.h"

#include "../tinythread/tinythread.h"

namespace cr
{

struct thread : object
{
	typedef tthread::thread thread_t;

	thread_t* thread_obj;

	static void _dstor(object* obj);
};

}	// namespace cr

#endif	// CR_THREAD_PRIVATE_H
