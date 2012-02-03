#include "../private/cr_application.h"

//#define _WIN32_WINNT 0x0500
//#define WINVER 0x0500
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h> // GET_X_LPARAM
#include <shlobj.h> // SHGetFolderPath
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );

void cr_app_main_invoker( void* ctx )
{
	cr_app_main();
}

int main( int argc, char** argv )
{
#if _DEBUG
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	_CrtSetBreakAlloc( -1 );
#endif

	HWND parentHWND = 0;
	int xres = 800;
	int yres = 480;
	bool hidden = false;

	for( int i = 0; i < argc; ++i )
	{

		if( strcmp( "-WaitForDebugger", argv[i] ) == 0 )
		{
			MessageBoxA( 0, "Waiting for debugger", "MCore", MB_OK );
		}
		else if( strcmp( "-ParentHWND", argv[i] ) == 0 )
		{
			parentHWND = ( HWND )atoi( argv[++i] );
		}
		else if( strcmp( "-Hidden", argv[i] ) == 0 )
		{
			hidden = true;
		}
		else if( strcmp( "-Res", argv[i] ) == 0 )
		{
			sscanf( argv[++i], "%dx%d", &xres, &yres );
		}
	}

	LPCSTR szName = "CrApp";
	WNDCLASSEXA wc = { sizeof( WNDCLASSEX ), CS_CLASSDC, MsgProc, 0L, 0L, GetModuleHandle( 0 ), 0, 0, 0, 0, szName, 0 };
	wc.hCursor = LoadCursor( 0, IDC_ARROW );
	RegisterClassExA( &wc );

	DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD dwStyle = WS_VISIBLE | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_OVERLAPPED;

	RECT rect;
	int windowWidth, windowHeight, windowLeft, windowTop;
	HWND hWnd;

	// process parentHWND
	if( 0 != parentHWND )
	{
		dwStyle = WS_CHILD | WS_VISIBLE;
		dwExStyle = 0;
	}

	if( hidden ) dwStyle &= ~WS_VISIBLE;

	// change current directoy to where the Exe is located
	{
		wchar_t path[MAX_PATH];
		wchar_t drive[_MAX_DRIVE];
		wchar_t dir[_MAX_DIR];
		wchar_t fname[_MAX_FNAME];
		wchar_t ext[_MAX_EXT];
		GetModuleFileNameW( GetModuleHandle( 0 ), path, MAX_PATH );
		_wsplitpath( path, drive, dir, fname, ext );
		_wmakepath( path, drive, dir, nullptr, nullptr );
		path[0] = path[0];
		if( FALSE == SetCurrentDirectoryW( path ) )
		{
			printf( "failed to change current directory to exe path" );
		}
	}

	// invoke startup callback
	cr_context_initialize();
	cr_app_startup();

	// create windows
	SetRect( &rect, 0, 0, xres, yres );
	AdjustWindowRectEx( &rect, dwStyle, FALSE, dwExStyle );
	if( nullptr == parentHWND )
	{
		windowWidth = rect.right - rect.left;
		windowHeight = rect.bottom - rect.top;
		windowLeft = GetSystemMetrics( SM_CXSCREEN ) / 2 - windowWidth / 2;
		windowTop = GetSystemMetrics( SM_CYSCREEN ) / 2 - windowHeight / 2;
	}
	else
	{
		windowWidth = rect.right - rect.left;
		windowHeight = rect.bottom - rect.top;
		windowLeft = 0;
		windowTop = 0;
	}
	hWnd = CreateWindowExA( 0, szName, szName, dwStyle, windowLeft, windowTop, windowWidth, windowHeight, parentHWND, 0, 0, 0 );
	SetWindowTextA( hWnd, "CrApp" );

	// start the main loop in a seperate thread
	cr_thread mainthread = cr_thread_new( nullptr, cr_app_main_invoker, nullptr );

	MSG msg = {0};
	while ( msg.message != WM_QUIT )
	{

		if ( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
		{
			// do some processing??
			Sleep( 0 );
		}
	}

	cr::application::inst.push_event( CR_APP_EVT_EXIT, nullptr );

	cr_thread_join( mainthread );

	cr_context_finalize();

	UnregisterClassA( szName, wc.hInstance );

	return 0;
}

LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg )
	{
	case WM_CLOSE:
	{
		PostQuitMessage( 0 );
		break;
	}
	}

	return DefWindowProc( hWnd, msg, wParam, lParam );
}
