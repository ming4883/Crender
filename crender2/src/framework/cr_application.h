#ifndef CR_APPLICATION_H
#define CR_APPLICATION_H

#include <crender.h>

#ifdef __cplusplus
extern "C" {
#endif

//!
struct cr_app_event
{
	struct cr_app_event* next;

	cr_uint32 type;
	void* value;
};

//! peek the next application-event from queue
cr_bool cr_app_peek_event(struct cr_app_event* evt);

//! remove the top application-event from queue
void cr_app_pop_event(void);

//! user implemented init function, called only once at application startup
void cr_app_startup(void);

//! user implemented main loop
void cr_app_main(void);

#ifdef __cplusplus
}
#endif

#endif  // CR_APPLICATION_H
