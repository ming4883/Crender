#include "private/cr_application.h"

namespace cr
{

application::application()
{
}

application::~application()
{
}

}	// namespace cr

cr_bool cr_app_peek_event(struct cr_app_event* evt)
{
	return CR_FALSE;
}

void cr_app_pop_event(void)
{
}
