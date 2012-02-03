#include <cr_application.h>

void cr_app_startup( void )
{
}

void cr_app_main( void )
{
	cr_app_event evt;

	while ( 1 )
	{
		if ( !cr_app_pop_event( &evt ) )
			continue;

		if ( CR_APP_EVT_EXIT == evt.type )
		{
			return;
		}
	}
}