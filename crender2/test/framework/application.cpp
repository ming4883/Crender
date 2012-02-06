#include <cr_application.h>

bool my_process_event( const cr_app_event& evt )
{
	if ( CR_APP_EVT_EXIT == evt.type )
		return false;

	return true;
}

void my_render( void )
{
	cr_gpu gpu = cr_app_gpu();

	cr_gpu_clear_color( gpu, 0.25f, 0.25f, 0.25f, 1.0f );

	cr_gpu_swap_buffers( gpu );
}

void cr_app_startup( void )
{
}

void cr_app_main( void )
{
	cr_app_event evt;

	while ( 1 )
	{
		if ( cr_app_pop_event( &evt ) && !my_process_event( evt ) )
			return;

		my_render();
	}
}