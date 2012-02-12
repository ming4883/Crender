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

	if ( !cr_gpu_begin( gpu ) )
		return;

	cr_uint32 xres, yres;

	cr_gpu_screen_resolution( gpu, &xres, &yres );

	cr_gpu_set_viewport( gpu, 0, 0, ( float )xres, ( float )yres, -1.0f, 1.0f );

	cr_gpu_clear_color( gpu, 0.25f, 0.25f, 0.25f, 1.0f );

	cr_gpu_swap_buffers( gpu );

	cr_gpu_end( gpu );
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