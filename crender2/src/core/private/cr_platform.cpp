#include "../cr_platform.h"

#include <stdio.h>
#include <stdarg.h>
#include "../nedmalloc/nedmalloc.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static LONGLONG cr_platform_time_impl( void )
{
	static LARGE_INTEGER frq;
	static bool frq_inited = false;

	if ( !frq_inited )
	{
		if ( QueryPerformanceFrequency( &frq ) )
			frq_inited = true;
	}

	LARGE_INTEGER now;
	if ( !QueryPerformanceCounter( &now ) )
		return 0;

	return ( now.QuadPart * 1000 ) / frq.QuadPart;
}

#else

static long cr_platform_time_impl( void )
{
	struct timespec now;
	clock_gettime( CLOCK_MONOTONIC, &now );
	return now.tv_sec * 1000 + now.tv_nsec / 1000;
}

#endif

#ifdef __cplusplus
extern "C" {
#endif

	//******************** memory ********************

#define CR_PRIVATE_USE_NEDMALLOC 1

#if CR_PRIVATE_USE_NEDMALLOC
#	define CR_PRIVATE_MALLOC nedmalloc
#	define CR_PRIVATE_FREE nedfree
#else
#	define CR_PRIVATE_MALLOC malloc
#	define CR_PRIVATE_FREE free
#endif

	CR_API static cr_mem_callback g_callback = {CR_PRIVATE_MALLOC, CR_PRIVATE_FREE};

	CR_API void cr_mem_set_callback( cr_mem_callback callback )
	{
		g_callback = callback;
		if ( nullptr == g_callback.alloc_fn || nullptr == g_callback.free_fn )
		{
			g_callback.alloc_fn = CR_PRIVATE_MALLOC;
			g_callback.free_fn = CR_PRIVATE_FREE;
		}
	}

	CR_API void* cr_mem_alloc( unsigned int size_in_bytes )
	{
		return g_callback.alloc_fn( size_in_bytes );
	}

	CR_API void cr_mem_free( void* ptr )
	{
		g_callback.free_fn( ptr );
	}

	//******************** print ********************

	CR_API void cr_default_print( const char* msg )
	{
		printf( "%s", msg );
	}

	static cr_print_func g_print_callback = cr_default_print;

	CR_API void cr_printf_set_callback( cr_print_func callback )
	{
		g_print_callback = callback;

		if ( nullptr == g_print_callback )
			g_print_callback = cr_default_print;
	}

	CR_API void cr_printf( const char* str, ... )
	{
		char msg[1024] = {0};

		va_list a;
		va_start( a, str );

		vsprintf ( msg, str, a );
		g_print_callback( msg );
	}

	//******************** time ********************

	CR_API int cr_platform_time_ms( void )
	{
		return ( int )cr_platform_time_impl();
	}

#ifdef __cplusplus
}
#endif