#ifndef CR_THREAD_PRIVATE_H
#define CR_THREAD_PRIVATE_H

#include "../cr_thread.h"
#include "cr_object.h"

#include "../tinythread/tinythread.h"

namespace cr
{

class tt_thread : public tthread::thread
{
public:
	CR_OVERRIDE_NEW_DELETE();
	tt_thread( void ( *func )( void * ), void * arg ) : tthread::thread( func, arg ) {}
};

class tt_mutex : public tthread::mutex
{
public:
	CR_OVERRIDE_NEW_DELETE();
};

class tt_lock_guard : public tthread::lock_guard<tt_mutex>
{
public:
	tt_lock_guard(tt_mutex& mutex) : tthread::lock_guard<tt_mutex>( mutex ) {}
};

struct thread : object
{
	CR_OVERRIDE_NEW_DELETE();

	tt_thread* thread_obj;

	thread( context* ctx, void ( *func )( void * ), void * arg );
	~thread( void );
};

}	// namespace cr

#endif	// CR_THREAD_PRIVATE_H
