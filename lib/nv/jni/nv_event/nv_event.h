//----------------------------------------------------------------------------------
// File:            libs\jni\nv_event\nv_event.h
// Samples Version: Android NVIDIA samples 2 
// Email:           tegradev@nvidia.com
// Forum:           http://developer.nvidia.com/tegra/forums/tegra-forums/android-development
//
// Copyright 2009-2010 NVIDIA® Corporation 
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//----------------------------------------------------------------------------------
#ifndef _NV_EVENT_H
#define _NV_EVENT_H
#include <sys/types.h>

/** @file nv_event.h
Contains a framework for event loop-based applications.  This library replaces
most or all of the normally-required JNI code for Android NDK applications,
presenting the application developer with two callbacks into which they can
write their application.  The framework runs in a natively-created thread,
allowing the application to implement a classic "event loop and render" structure
without having to return to Java code to avoid ANR warnings.  The library 
includes support for input and system event passing as well as JNI initialization
and exposes basic EGL functionality to native as well.  Put together, the library
can form the basis of a simple interactive 3D application.  All applications that
are based on this library must also be subclassed on the Java side from the
provided NvEventQueueActivity.  Additional external documentation on the use of
this library, the related Java code and the tool provided to create a template
application based on the library are provided with the SDK.
@see NvEventQueueActivity
*/

#ifdef ANDROID

#include <jni.h>
typedef jobject NVEventPlatformAppHandle;

#else // unknown platform

typedef void* NVEventPlatformAppHandle;

#endif


enum
{
/** Timeout argument to NVEventGetNextEvent() that indicates the function should 
	block until there is an event pending or the app exits 
	@see NVEventGetNextEvent()
	*/
	NV_EVENT_WAIT_FOREVER = -1
};

/** Event type values
*/
typedef enum NVEventType
{
	/** Key up/down events */
	NV_EVENT_KEY = 1,
	/** Translated character events */
	NV_EVENT_CHAR,
	/** Single-touch pointer events */
	NV_EVENT_TOUCH,
	/** Multi-touch events */
	NV_EVENT_MULTITOUCH,
	/** Accelerometer events */
	NV_EVENT_ACCEL,

	// The events below, up to resume are special, because they relate to the app's status
	// they must be preserved in the framework's internal code. Please update
	// isSacred if this is changed.
	/** Window size changed events */
	NV_EVENT_WINDOW_SIZE,
	/** Quit request events */
	NV_EVENT_QUIT,
	/** the app should release all graphics resources and block waiting for
	 * RESTART or QUIT after receiving this event */
	NV_EVENT_PAUSE,
	/** the app should block waiting for RESUME OR QUIT after doing all non-graphics
	 * initialization and initalize graphics resources after receiving RESTART */
	NV_EVENT_RESUME,

	/* a dummy enum value used to compute num_events */
	NV_EVENT_NUM_EVENT_DUMMY_DONTUSEME,
	/* total number of events */
	NV_EVENT_NUM_EVENTS = NV_EVENT_NUM_EVENT_DUMMY_DONTUSEME - 1,
	NV_EVENT_FORCE_32BITS = 0x7fffffff
} NVEventType;

/** Touch event actions
*/
typedef enum NVTouchEventType
{
	/** Pointer has just left the screen */
	NV_TOUCHACTION_UP,
	/** Pointer has just gone down onto the screen */
	NV_TOUCHACTION_DOWN,
	/** Pointer is moving on the screen */
	NV_TOUCHACTION_MOVE,
	NV_TOUCHACTION_FORCE_32BITS = 0x7fffffff
} NVTouchEventType;

/** Multitouch event flags 
*/
typedef enum NVMultiTouchEventType
{
	/** Indicated pointers are leaving the screen */
	NV_MULTITOUCH_UP    =   0x00000001,
	/** Indicated pointers have just touched the screen */
	NV_MULTITOUCH_DOWN  =   0x00000002,
	/** Indicated pointers are moving on the screen */
	NV_MULTITOUCH_MOVE  =   0x00000003,
	/** Indicated pointers have halted the current gesture
		app should cancel any actions implied by the gesture */
	NV_MULTITOUCH_CANCEL =  0x00000004,
	/** Mask to be AND'ed with the flag value 
		to get the active pointer bits */
	NV_MULTITOUCH_POINTER_MASK =  0x0000ff00,
	/** Number of bits to right-shift the masked value 
		to get the active pointer bits */
	NV_MULTITOUCH_POINTER_SHIFT = 0x00000008,
	/** Mask to be AND'ed with the flag value 
		to get the event action */
	NV_MULTITOUCH_ACTION_MASK =   0x000000ff,
	NV_MULTITOUCH_FORCE_32BITS = 0x7fffffff
} NVMultiTouchEventType;

/** Key event types
*/
typedef enum NVKeyEventType
{
	/** Key has just been pressed (no repeats) */
	NV_KEYACTION_UP,
	/** Key has just been release */
	NV_KEYACTION_DOWN,
	NV_KEYACTION_FORCE_32BITS = 0x7fffffff
} NVKeyEventType;

/** Key event key codes
*/
typedef enum NVKeyCode
{
	NV_KEYCODE_NULL = 0,
	NV_KEYCODE_BACK,
	NV_KEYCODE_TAB,
	NV_KEYCODE_ENTER,
	NV_KEYCODE_DEL,
	NV_KEYCODE_SPACE,
	NV_KEYCODE_ENDCALL,
	NV_KEYCODE_HOME,

	NV_KEYCODE_STAR,
	NV_KEYCODE_PLUS,
	NV_KEYCODE_MINUS,
	NV_KEYCODE_NUM,

	NV_KEYCODE_DPAD_LEFT,
	NV_KEYCODE_DPAD_UP,
	NV_KEYCODE_DPAD_RIGHT,
	NV_KEYCODE_DPAD_DOWN,

	NV_KEYCODE_0,
	NV_KEYCODE_1,
	NV_KEYCODE_2,
	NV_KEYCODE_3,
	NV_KEYCODE_4,
	NV_KEYCODE_5,
	NV_KEYCODE_6,
	NV_KEYCODE_7,
	NV_KEYCODE_8,
	NV_KEYCODE_9,

	NV_KEYCODE_A,
	NV_KEYCODE_B,
	NV_KEYCODE_C,
	NV_KEYCODE_D,
	NV_KEYCODE_E,
	NV_KEYCODE_F,
	NV_KEYCODE_G,
	NV_KEYCODE_H,
	NV_KEYCODE_I,
	NV_KEYCODE_J,
	NV_KEYCODE_K,
	NV_KEYCODE_L,
	NV_KEYCODE_M,
	NV_KEYCODE_N,
	NV_KEYCODE_O,
	NV_KEYCODE_P,
	NV_KEYCODE_Q,
	NV_KEYCODE_R,
	NV_KEYCODE_S,
	NV_KEYCODE_T,
	NV_KEYCODE_U,
	NV_KEYCODE_V,
	NV_KEYCODE_W,
	NV_KEYCODE_X,
	NV_KEYCODE_Y,
	NV_KEYCODE_Z,

	NV_KEYCODE_ALT_LEFT,
	NV_KEYCODE_ALT_RIGHT,

	NV_KEYCODE_SHIFT_LEFT,
	NV_KEYCODE_SHIFT_RIGHT,

	NV_KEYCODE_APOSTROPHE,
	NV_KEYCODE_SEMICOLON,
	NV_KEYCODE_EQUALS,
	NV_KEYCODE_COMMA,
	NV_KEYCODE_PERIOD,
	NV_KEYCODE_SLASH,
	NV_KEYCODE_GRAVE,
	NV_KEYCODE_BACKSLASH,

	NV_KEYCODE_LEFT_BRACKET,
	NV_KEYCODE_RIGHT_BRACKET,

	NV_KEYCODE_FORCE_32BIT = 0x7fffffff
} NVKeyCode;

/** Single-touch event data
*/
typedef struct NVEventTouch
{
	/** The action code */
    NVTouchEventType   m_action;
	/** The window-relative X position (in pixels) */
    float   m_x;
	/** The window-relative Y position (in pixels) */
    float   m_y;
} NVEventTouch;

/** Multi-touch event data
*/
typedef struct NVEventMultiTouch
{
	/** The action flags */
    NVMultiTouchEventType   m_action;
	/** The window-relative X position of the first pointer (in pixels) 
		only valid if bit 0 of the pointer bits is set */
    float   m_x1;
	/** The window-relative Y position of the first pointer (in pixels) 
		only valid if bit 0 of the pointer bits is set */
    float   m_y1;
	/** The window-relative X position of the second pointer (in pixels) 
		only valid if bit 1 of the pointer bits is set */
    float   m_x2;
	/** The window-relative Y position of the second pointer (in pixels) 
		only valid if bit 1 of the pointer bits is set */
    float   m_y2;
} NVEventMultiTouch;

/** Key down/up event data
*/
typedef struct NVEventKey
{
	/** The action flags */
	NVKeyEventType m_action;
	/** The code of the key pressed or released */
	NVKeyCode m_code;
} NVEventKey;

/** Translated character event data
*/
typedef struct NVEventChar
{
	/** The UNICODE character represented */
	int32_t m_unichar;
} NVEventChar;

/** Accelerometer event data
*/
typedef struct NVEventAccel
{
	/** Signed X magnitude of the force vector */
    float   m_x;
	/** Signed Y magnitude of the force vector */
    float   m_y;
	/** Signed Z magnitude of the force vector */
    float   m_z;
} NVEventAccel;

/** Windows size change event data
*/
typedef struct NVEventWindowSize
{
	/** New window client area width (in pixels) */
	int32_t m_w;
	/** New window client area height (in pixels) */
	int32_t m_h;
} NVEventWindowSize;

/** All-encompassing event structure 
*/
typedef struct NVEvent 
{
	/** The type of the event, which also indicates which m_data union holds the data */
    NVEventType m_type;
	/** Union containing all possible event type data */
    union NVEventData 
	{
		/** Data for single-touch events */
		NVEventTouch m_touch;
		/** Data for multi-touch events */
		NVEventMultiTouch m_multi;
		/** Data for key up/down events */
        NVEventKey m_key;
		/** Data for charcter events */
		NVEventChar m_char;
		/** Data for accelerometer events */
        NVEventAccel m_accel;
		/** Data for window size events */
		NVEventWindowSize m_size;
    } m_data;
} NVEvent;

/** Returns a string describing the event
@param eventType The event type
@return Returns a string containing a description of the event. Do not free or delete this memory.
@see NVEvent */
const char* NVEventGetEventStr(NVEventType eventType);


/** Returns Android's current rotation value
@return Returns Android's current rotation value
@see NVEvent */
int NVEventGetOrientation();

/** Returns the next pending event for the application to process.  Can return immediately if there
is no event, or can wait a fixed number of milisecs (or "forever") if desired.
@param ev A pointer to a pre-allocated NVEvent structure to be filled with the pending event
@param waitMSecs The maximum time (in milisecs) to wait for an event before returning "no event".  
	Pass NV_EVENT_WAIT_FOREVER to wait indefinitely for an event.  Note that NV_EVENT_WAIT_FOREVER
	does not gaurantee an event on return.  The function can still return on error or if the
	app is exiting. Default is to return immediately, event or not. 
@return Nonzero if an event was pending and written into ev, zero if no event was
	pending in the requested timeout period
@see NVEvent */
int32_t NVEventGetNextEvent(NVEvent* ev, int waitMSecs = 0);

/** Returns when one of the events in the eventTypes array is placed into the
event queue.
@param eventTypes A pointer to an array of event ids
@param numEvents The size of the eventTypes array
@return Nonzero if the requested event is pending, else zero
@see NVEvent */
int32_t NVEventWaitForEvents(const int* eventTypes, int numEvents, int waitMSecs);

/** Optional default event function. It will print messages about unhandled events,
for instance, if an event is required to be specially handled and it is not.
@param event The unhandled event */
void NVEventUnhandledEvent(NVEvent* ev);

/** The app-supplied "callback" for initialization during JNI_OnLoad.  
Declares the application's pre-main initialization function.  Does not define the
function.  <b>The app must define this in its own code<b>, even if the function is empty.
JNI init code can be safely called from here, as it WILL be called from
within a JNI function thread
@parm argc Passes the number of command line arguments.  
	This is currently unsupported and is always passed 0 
@parm argv Passes the array of command line arguments.  
	This is currently unsupported and is always passed NULL 
@return The function should return 0 on success and nonzero on failure.
*/
extern int32_t NVEventAppInit(int32_t argc, char** argv);

/** The app-supplied "callback" for running the application's main loop.  
Declares the application's main loop function.  Does not define the
function.  <b>The app must define this in its own code</b>
This function will be spawned in its own thread and should not return until
the application receives an NV_EVENT_QUIT from NVEventGetNextEvent()
At the outset of this function, there is an EGL context/surface pair available to be
bound, but they are not initially bound for the thread.
@parm argc Passes the number of command line arguments.  
	This is currently unsupported and is always passed 0 
@parm argv Passes the array of command line arguments.  
	This is currently unsupported and is always passed NULL 
@return The function should return 0 on success and nonzero on failure.  The function
should run an event loop and not return until reuqested to exit with an NV_EVENT_QUIT event
*/
extern int32_t NVEventAppMain(int32_t argc, char** argv);

/** Initializes EGL and GLES2 for rendering */
bool NVEventEGLInit();

/** Swaps the currently-bound surface (if one is bound) */
void NVEventEGLSwapBuffers();
/** Binds the EGL context and surface to the calling thread.  The calling thread must
have been created with nv_thread, so that it is bound to the JVM.  The context must
not be concurrently bound to any other thread */
void NVEventEGLMakeCurrent();
/** Unbinds the context and surface from the current thread for use in another thread or for shutdown
The calling thread must have the context bound at the time of the call, and thus must also have been
created with nv_thread, so that it is bound to the JVM */
void NVEventEGLUnmakeCurrent();

/** Finds and loads the named texture from the application's APK or the device's /data folder,
returning the width, height and format information, as well as a pointer to the pixels.  The
pixel data can be loaded to OpenGL ES via glTexImage2D.  When the application is done loading the
texture, it MUST call NVEventReleaseTextureData() with the handle and pixels to avoid leaving resources
@param filename The string name of the image file to be found and loaded.  Supported formats include PNG
and JPG.
@param pixels (OUT) Returns a pointer to a block of pixel data allocated by the function and filled with
the given image
@param width (OUT) The width of the image in pixels
@param height (OUT) The height of the image in pixels
@param format (OUT) The GL format of the image (e.g. GL_RGB)
@param type (OUT) The GL type of the image (e.g. GL_UNSIGNED_BYTE)
@return An opaque handle to the texture resources to be passed to NVEventReleaseTextureData().  
Returns NULL on failure */
void* NVEventGetTextureData(const char* filename, unsigned char*& pixels, 
							unsigned int& width, unsigned int& height,
							unsigned int& format, unsigned int& type);

/** Releases the resources loaded in a successful call to NVEventGetTextureData()
@param data The handle returned from the call to NVEventGetTextureData()
@param pixels The pixel pointer returned from the matching call to NVEventGetTextureData() */
void NVEventReleaseTextureData(void* data, unsigned char* pixels);

/** Returns the platform-specific handle to the application instance, if supported.  This
function is, by definition platform-specific.
@return A platform-specific handle to the application.  */

NVEventPlatformAppHandle NVEventGetPlatformAppHandle();

//#define _TIMFUDGE

#ifdef _TIMFUDGE
	int NVEventEGLGetSurfaceWidth();
	int NVEventEGLGetSurfaceHeight();
#endif

#endif
