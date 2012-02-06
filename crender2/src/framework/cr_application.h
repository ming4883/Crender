#ifndef CR_APPLICATION_H
#define CR_APPLICATION_H

#include <crender.h>

#ifdef __cplusplus
extern "C" {
#endif

	/*! structure of an application event.
		type describes the type of the event using a cr_strhash
		value acts as a generic buffer to hold the event data
	*/
	struct cr_app_event
	{
		cr_uint32 type;
		cr_uint8 value[64];
	};

#define CR_APP_EVT_EXIT cr_strhash("app_exit")		//!< the application is exiting
#define CR_APP_EVT_RESIZE cr_strhash("app_resize")	//!< the application is resized, value contains cr_uint32[2]
#define CR_APP_EVT_PAUSE cr_strhash("app_pause")	//!< the application is paused
#define CR_APP_EVT_RESUME cr_strhash("app_resume")	//!< the application is resumed

	/*! pop the top event from application event queue
		\param evt, pointer to an cr_app_event struct to recieve the event
	*/
	cr_bool cr_app_pop_event( struct cr_app_event* evt );

	/*! get the application-wide gpu object
	*/
	cr_gpu cr_app_gpu( void );

//! user implemented init function, called only once at application startup
	void cr_app_startup( void );

//! user implemented main loop
	void cr_app_main( void );

#ifdef __cplusplus
}
#endif

#endif  // CR_APPLICATION_H
