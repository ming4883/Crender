#include "Framework.h"
#include "API.gl.h"
#include <sys/time.h>

#include <jni.h>
#include <errno.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

/**
* Shared state for our app.
*/
struct engine
{
	struct android_app* app;

	ASensorManager* sensorManager;
	const ASensor* accelerometerSensor;
	ASensorEventQueue* sensorEventQueue;

	EGLDisplay display;
	EGLSurface surface;
	EGLContext context;
	
	long previousTime;
};

long engine_get_time() 
{
	struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec*1000000 + now.tv_nsec/1000;
}

/**
* Initialize an EGL context for the current display.
*/
static int engine_init_display(struct engine* engine)
{

	const EGLint configAttribs[] = {
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_BLUE_SIZE, 5,
			EGL_GREEN_SIZE, 6,
			EGL_RED_SIZE, 5,
			EGL_DEPTH_SIZE, 16,
			EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
			EGL_NONE
	};
	
	const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2,	EGL_NONE};
	
	EGLint w, h, dummy, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;

	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	eglInitialize(display, 0, 0);
	eglChooseConfig(display, configAttribs, &config, 1, &numConfigs);
	
	if (0 == numConfigs) {
		crDbgStr("eglChooseConfig return %d configs", numConfigs);
		return -1;
	}

	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
	ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

	surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
	context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
		crDbgStr("Unable to eglMakeCurrent");
		return -1;
	}

	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	engine->display = display;
	engine->context = context;
	engine->surface = surface;
	
	engine->previousTime = engine_get_time();
	
	crAppContext.xres = w;
	crAppContext.yres = h;

	// Initialize GL state.
	crAppInitialize();

	return 0;
}

/**
* Just the current frame in the display.
*/
static void engine_draw_frame(struct engine* engine)
{
	long currentTime = engine_get_time();
	long deltaTime = currentTime - engine->previousTime;
	engine->previousTime = currentTime;
	
	if (engine->display == NULL) {
		// No display.
		return;
	}
	
	crAppUpdate(deltaTime / 1000);
	crAppRender();
	
	{
		GLint err = glGetError();
		if(err != GL_NO_ERROR) {
			crDbgStr("GL has error %x", err);
		}
	}

	eglSwapBuffers(engine->display, engine->surface);
	
	
}

/**
* Tear down the EGL context currently associated with the display.
*/
static void engine_term_display(struct engine* engine)
{
	crAppFinalize();
	if (engine->display != EGL_NO_DISPLAY) {
		eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (engine->context != EGL_NO_CONTEXT) {
			eglDestroyContext(engine->display, engine->context);
		}
		if (engine->surface != EGL_NO_SURFACE) {
			eglDestroySurface(engine->display, engine->surface);
		}
		eglTerminate(engine->display);
	}
	engine->display = EGL_NO_DISPLAY;
	engine->context = EGL_NO_CONTEXT;
	engine->surface = EGL_NO_SURFACE;
}

/**
* Process the next input event.
*/
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
	struct engine* engine = (struct engine*)app->userData;
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
		//engine->state.x = AMotionEvent_getX(event, 0);
		//engine->state.y = AMotionEvent_getY(event, 0);
		return 1;
	}
	return 0;
}

/**
* Process the next main command.
*/
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
	struct engine* engine = (struct engine*)app->userData;
	switch (cmd) {
		case APP_CMD_SAVE_STATE:
			// The system has asked us to save our current state.  Do so.
			//engine->app->savedState = malloc(sizeof(struct saved_state));
			//*((struct saved_state*)engine->app->savedState) = engine->state;
			//engine->app->savedStateSize = sizeof(struct saved_state);
			break;
		case APP_CMD_INIT_WINDOW:
			// The window is being shown, get it ready.
			if (engine->app->window != NULL) {
				engine_init_display(engine);
				engine_draw_frame(engine);
			}
			break;
		case APP_CMD_TERM_WINDOW:
			// The window is being hidden or closed, clean it up.
			engine_term_display(engine);
			break;
		case APP_CMD_GAINED_FOCUS:
			// When our app gains focus, we start monitoring the accelerometer.
			if (engine->accelerometerSensor != NULL) {
				ASensorEventQueue_enableSensor(engine->sensorEventQueue,
						engine->accelerometerSensor);
				// We'd like to get 60 events per second (in us).
				ASensorEventQueue_setEventRate(engine->sensorEventQueue,
						engine->accelerometerSensor, (1000L/60)*1000);
			}
			break;
		case APP_CMD_LOST_FOCUS:
			// When our app loses focus, we stop monitoring the accelerometer.
			// This is to avoid consuming battery while not being used.
			if (engine->accelerometerSensor != NULL) {
				ASensorEventQueue_disableSensor(engine->sensorEventQueue,
						engine->accelerometerSensor);
			}
			// Also stop animating.
			engine_draw_frame(engine);
			break;
	}
}

struct android_app* CR_ANDROID_APP = 0;

CrAppContext crAppContext = {
	"crApp",
	"gles",
	2, 0,
	CrFalse,
	CrFalse,
	800,
	480,
};

void* crOpen(const char* filename)
{
	AAsset* asset = AAssetManager_open(CR_ANDROID_APP->activity->assetManager, filename, AASSET_MODE_UNKNOWN);

	if(nullptr == asset) {
		crDbgStr("failed to open asset '%s'!\n", filename);
	}
	return asset;
}

void crClose(void* handle)
{
	AAsset_close((AAsset*)handle);
}

size_t crRead(void* buff, size_t elsize, size_t nelem, void* handle)
{
	return (size_t)AAsset_read((AAsset*)handle, buff, elsize * nelem);
}

/**
* This is the main entry point of a native application that is using
* android_native_app_glue.  It runs in its own thread, with its own
* event loop for receiving input events and doing other things.
*/
void android_main(struct android_app* state) {
	struct engine engine;
	
	CR_ANDROID_APP = state;
	
	crAppConfig();

	// Make sure glue isn't stripped.
	app_dummy();

	memset(&engine, 0, sizeof(engine));
	state->userData = &engine;
	state->onAppCmd = engine_handle_cmd;
	state->onInputEvent = engine_handle_input;
	engine.app = state;

	// Prepare to monitor accelerometer
	engine.sensorManager = ASensorManager_getInstance();
	engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,
			ASENSOR_TYPE_ACCELEROMETER);
	engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager,
			state->looper, LOOPER_ID_USER, NULL, NULL);

	if (state->savedState != NULL) {
		// We are starting with a previous saved state; restore from it.
		//engine.state = *(struct saved_state*)state->savedState;
	}

	// loop waiting for stuff to do.

	while (1) {
		// Read all pending events.
		int ident;
		int events;
		struct android_poll_source* source;

		// If not animating, we will block forever waiting for events.
		// If animating, we loop until all events are read, then continue
		// to draw the next frame of animation.
		while ((ident=ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0) {

			// Process this event.
			if (source != NULL) {
				source->process(state, source);
			}

			// If a sensor has data, process it now.
			if (ident == LOOPER_ID_USER) {
				if (engine.accelerometerSensor != NULL) {
					ASensorEvent event;
					while (ASensorEventQueue_getEvents(engine.sensorEventQueue,
							&event, 1) > 0) {
					}
				}
			}

			// Check if we are exiting.
			if (state->destroyRequested != 0) {
				engine_term_display(&engine);
				return;
			}
		}

		engine_draw_frame(&engine);
	}
}
