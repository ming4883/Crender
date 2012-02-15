#include "private/cr_gpu.win32.h"
#include "private/cr_context.h"
#include <GL/wglew.h>

namespace cr
{

win32_gpu::win32_gpu( context* ctx )
	: gpu( ctx )
{
	for ( int i = 0; i < CMD_QUEUE_COUNT; ++i )
	{
		queues[i].cmd_queue = cr_command_queue_new( ( cr_context )ctx );
		queues[i].status = queue::EMPTY;
	}
	feeding_queue = nullptr;
}

win32_gpu::~win32_gpu( void )
{
	for ( int i = 0; i < CMD_QUEUE_COUNT; ++i )
	{
		cr_release( queues[i].cmd_queue );
	}
}

void win32_gpu::init( void** window, struct cr_gpu_desc* desc )
{
	HWND* hWnd = ( HWND* )window;
	WINDOWINFO info;

	GLenum err;
	PIXELFORMATDESCRIPTOR pfd;
	HDC hDC;
	HGLRC hRC;
	int pixelFormat;
	char glVersionStr[64];

	GetWindowInfo( *hWnd, &info );
	this->xres = info.rcClient.right - info.rcClient.left;
	this->yres = info.rcClient.bottom - info.rcClient.top;

	// Create the GL context.
	ZeroMemory( &pfd, sizeof( pfd ) );
	pfd.nSize = sizeof( pfd );
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.iLayerType = PFD_MAIN_PLANE;

	hDC = GetDC( *hWnd );
	pixelFormat = ChoosePixelFormat( hDC, &pfd );

	SetPixelFormat( hDC, pixelFormat, &pfd );
	hRC = wglCreateContext( hDC );
	wglMakeCurrent( hDC, hRC );

	if ( desc->msaa_level > 1 )
	{
		int pixelAttribs[] =
		{
			WGL_SAMPLES_ARB, desc->msaa_level,
			WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_RED_BITS_ARB, 8,
			WGL_GREEN_BITS_ARB, 8,
			WGL_BLUE_BITS_ARB, 8,
			WGL_ALPHA_BITS_ARB, 8,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			0
		};
		int* sampleCount = pixelAttribs + 1;
		int* useSampleBuffer = pixelAttribs + 3;
		int pixelFormat = -1;
		PROC proc = wglGetProcAddress( "wglChoosePixelFormatARB" );
		unsigned int numFormats;
		PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = ( PFNWGLCHOOSEPIXELFORMATARBPROC ) proc;

		if ( !wglChoosePixelFormatARB )
		{
			cr_dbg_str( "Could not load function pointer for 'wglChoosePixelFormatARB'.  Is your driver properly installed?" );
		}

		// Try fewer and fewer samples per pixel till we find one that is supported:
		while ( pixelFormat <= 0 && *sampleCount >= 0 )
		{
			wglChoosePixelFormatARB( hDC, pixelAttribs, 0, 1, &pixelFormat, &numFormats );
			( *sampleCount )--;
			if ( *sampleCount <= 1 )
			{
				*useSampleBuffer = GL_FALSE;
			}
		}

		// Win32 allows the pixel format to be set only once per app, so destroy and re-create the app:
		{

			char szName[64];
			int windowWidth, windowHeight, windowLeft, windowTop;

			GetClassName( *hWnd, szName, 64 );

			windowWidth = info.rcWindow.right - info.rcWindow.left;
			windowHeight = info.rcWindow.bottom - info.rcWindow.top;
			windowLeft = info.rcWindow.left;
			windowTop = info.rcWindow.top;

			DestroyWindow( *hWnd );

			*hWnd = CreateWindowExA( info.dwExStyle, szName, szName, info.dwStyle,
									 windowLeft, windowTop, windowWidth, windowHeight,
									 0, 0, 0, 0 );

			SetWindowPos( *hWnd, HWND_TOP, windowLeft, windowTop, windowWidth, windowHeight, 0 );
			hDC = GetDC( *hWnd );

			SetPixelFormat( hDC, pixelFormat, &pfd );
			hRC = wglCreateContext( hDC );
			wglMakeCurrent( hDC, hRC );
		}
	}

	err = glewInit();
	if ( GLEW_OK != err )
	{
		cr_dbg_str( "GLEW Error: %s\n", glewGetErrorString( err ) );
	}
	cr_dbg_str( "OpenGL Version: %s\n", glGetString( GL_VERSION ) );

	if ( !desc->vsync )
		wglSwapIntervalEXT( 0 );

	sprintf( glVersionStr, "GL_VERSION_%d_%d", desc->api_major, desc->api_minor );
	if ( ( desc->api_major > 2 ) && glewIsSupported( glVersionStr ) )
	{
		const int contextAttribs[] =
		{
			WGL_CONTEXT_MAJOR_VERSION_ARB, desc->api_major,
			WGL_CONTEXT_MINOR_VERSION_ARB, desc->api_minor,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
			0
		};

		HGLRC newRC = wglCreateContextAttribsARB( hDC, 0, contextAttribs );
		wglMakeCurrent( 0, 0 );
		wglDeleteContext( hRC );
		hRC = newRC;
		wglMakeCurrent( hDC, hRC );

		// 3.0 or above requires a vertex array
		glGenVertexArrays( 1, &this->gl_vtx_array_name );
		glBindVertexArray( this->gl_vtx_array_name );
	}

	this->hdc = ( GLuint )hDC;
	this->hrc = ( GLuint )hRC;
	this->hwnd = *hWnd;
}

void win32_gpu::swap_buffer( cr_command_queue cmd_queue, void* a )
{
	float16_args* args = ( float16_args* )a;

	SwapBuffers( ( HDC )args->self->hdc );

	cr_mem_free( args );
}

void win32_gpu::set_viewport( cr_command_queue cmd_queue, void* a )
{
	float16_args* args = ( float16_args* )a;

	glEnable(GL_SCISSOR_TEST);
	glViewport( ( GLint )args->value[0], ( GLint )args->value[1], ( GLint )args->value[2], ( GLint )args->value[3] );
	glScissor( ( GLint )args->value[0], ( GLint )args->value[1], ( GLint )args->value[2], ( GLint )args->value[3] );
	glDepthRange( args->value[4], args->value[5] );

	cr_mem_free( args );
}

void win32_gpu::clear_color( cr_command_queue cmd_queue, void* a )
{
	float16_args* args = ( float16_args* )a;

	glClearColor( args->value[0], args->value[1], args->value[2], args->value[3] );
	glClear( GL_COLOR_BUFFER_BIT );

	cr_mem_free( args );
}

void win32_gpu::clear_depth( cr_command_queue cmd_queue, void* a )
{
	float16_args* args = ( float16_args* )a;

	//if ( CR_FALSE == impl->appliedGpuState.depthWrite )
	//	glDepthMask(GL_TRUE);

	glClearDepth( args->value[0] );
	glClear( GL_DEPTH_BUFFER_BIT );

	//if ( CR_FALSE == impl->appliedGpuState.depthWrite )
	//	glDepthMask(GL_FALSE);

	cr_mem_free( args );
}

}	// namespace cr

#ifdef __cplusplus
extern "C" {
#endif

	typedef cr::win32_gpu gpu_t;

	CR_API cr_gpu cr_gpu_new( cr_context context, void** window, struct cr_gpu_desc* desc )
	{
		CR_ASSERT( cr::context::singleton );

		gpu_t* self = new gpu_t( cr_context_get( context ) );
		self->init( window, desc );

		return ( cr_gpu )self;
	}

	CR_API void cr_gpu_flush( cr_gpu s )
	{
		gpu_t* self = ( gpu_t* )s;

		if ( nullptr == self->feeding_queue || gpu_t::queue::FULL != self->feeding_queue->status )
			return;

		gpu_t::queue* queue = self->feeding_queue;
		self->feeding_queue = nullptr;

		cr_command_queue_consume_all( queue->cmd_queue);
		queue->status = gpu_t::queue::EMPTY;
	}

	CR_API cr_bool cr_gpu_begin( cr_gpu s )
	{
		gpu_t* self = ( gpu_t* )s;

		if ( nullptr != self->feeding_queue )
			return CR_FALSE;

		for ( int i = 0; i < gpu_t::CMD_QUEUE_COUNT; ++i )
		{
			if ( gpu_t::queue::EMPTY == self->queues[i].status )
			{
				self->feeding_queue = &self->queues[i];
				self->feeding_queue->status = gpu_t::queue::FEEDING;
				return CR_TRUE;
			}
		}

		return CR_FALSE;
	}

	CR_API void cr_gpu_end( cr_gpu s )
	{
		gpu_t* self = ( gpu_t* )s;

		if ( nullptr == self->feeding_queue )
			return;

		self->feeding_queue->status = gpu_t::queue::FULL;
	}

	CR_API void cr_gpu_swap_buffers( cr_gpu s )
	{
		gpu_t* self = ( gpu_t* )s;
		gpu_t::float16_args* args = ( gpu_t::float16_args* )cr_mem_alloc( sizeof( gpu_t::float16_args ) );
		args->self = self;
		cr_command_queue_produce( self->feeding_queue->cmd_queue, gpu_t::swap_buffer, args );
	}

	CR_API void cr_gpu_set_viewport( cr_gpu s, float x, float y, float w, float h, float zmin, float zmax )
	{
		gpu_t* self = ( gpu_t* )s;
		gpu_t::float16_args* args = ( gpu_t::float16_args* )cr_mem_alloc( sizeof( gpu_t::float16_args ) );
		args->self = self;
		args->value[0] = x;
		args->value[1] = y;
		args->value[2] = w;
		args->value[3] = h;
		args->value[4] = zmin;
		args->value[5] = zmax;
		cr_command_queue_produce( self->feeding_queue->cmd_queue, gpu_t::set_viewport, args );
	}

	CR_API void cr_gpu_clear_color( cr_gpu s, float r, float g, float b, float a )
	{
		gpu_t* self = ( gpu_t* )s;
		gpu_t::float16_args* args = ( gpu_t::float16_args* )cr_mem_alloc( sizeof( gpu_t::float16_args ) );
		args->self = self;
		args->value[0] = r;
		args->value[1] = g;
		args->value[2] = b;
		args->value[3] = a;
		cr_command_queue_produce( self->feeding_queue->cmd_queue, gpu_t::clear_color, args );
	}

	CR_API void cr_gpu_clear_depth( cr_gpu s, float z )
	{
		gpu_t* self = ( gpu_t* )s;
		gpu_t::float16_args* args = ( gpu_t::float16_args* )cr_mem_alloc( sizeof( gpu_t::float16_args ) );
		args->self = self;
		args->value[0] = z;
		cr_command_queue_produce( self->feeding_queue->cmd_queue, gpu_t::clear_depth, args );
	}

#ifdef __cplusplus
}
#endif
