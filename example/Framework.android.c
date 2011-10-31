//BEGIN_INCLUDE(all)
#include <jni.h>
#include <errno.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "crframework", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "crframework", __VA_ARGS__))

#include "Framework.h"

struct android_app* CR_ANDROID_APP;

CrAppContext crAppContext = {
	"crApp", nullptr
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

void crSleep(unsigned int milliseconds)
{
}

/**
 * Our saved state data.
 */
struct saved_state {
	int dummy;
};

/**
 * Shared state for our app.
 */
struct Engine {
	struct android_app* app;

	ASensorManager* sensorManager;
	const ASensor* accelerometerSensor;
	ASensorEventQueue* sensorEventQueue;

	int animating;
	long lastTime;
	struct saved_state state;
};

static long engine_get_time() 
{
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return now.tv_sec*1000000 + now.tv_nsec/1000;
}

/**
 * Initialize an EGL context for the current display.
 */
static int engine_init_display(struct Engine* engine) {
	
	if(CrFalse == crContextInit(crAppContext.context, (void**)&engine->app->window)) {
		LOGW("Unable to initialize crContext");
		return -1;
	}
	
	LOGI("crContext initialized");
	crAppInitialize();
	
	engine->animating = 1;
	engine->lastTime = engine_get_time();

	return 0;
}

/**
 * Just the current frame in the display.
 */
static void engine_draw_frame(struct Engine* engine) {

	crContextPreRender(crAppContext.context);

	crAppRender();

	crContextPostRender(crAppContext.context);

	crContextSwapBuffers(crAppContext.context);
}

/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display(struct Engine* engine) {
	crContextFree(crAppContext.context);
	engine->animating = 0;
}

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
	struct Engine* engine = (struct Engine*)app->userData;
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
		int x, y;
		engine->animating = 1;
		x = AMotionEvent_getX(event, 0);
		y = AMotionEvent_getY(event, 0);
		crAppHandleMouse(x, y, CrApp_MouseDown);
		return 1;
	}
	return 0;
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd) {
	struct Engine* engine = (struct Engine*)app->userData;
	switch (cmd) {
		case APP_CMD_SAVE_STATE:
			// The system has asked us to save our current state.  Do so.
			engine->app->savedState = malloc(sizeof(struct saved_state));
			*((struct saved_state*)engine->app->savedState) = engine->state;
			engine->app->savedStateSize = sizeof(struct saved_state);
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
			engine->animating = 0;
			engine_draw_frame(engine);
			break;
	}
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state) {
	struct Engine engine;

	// Make sure glue isn't stripped.
	app_dummy();
	
	CR_ANDROID_APP = state;

	crAppContext.context = crContextAlloc();
	crAppConfig();

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
		engine.state = *(struct saved_state*)state->savedState;
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
		while ((ident=ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events,
				(void**)&source)) >= 0) {

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
						//LOGI("accelerometer: x=%f y=%f z=%f",
						//        event.acceleration.x, event.acceleration.y,
						//        event.acceleration.z);
					}
				}
			}

			// Check if we are exiting.
			if (state->destroyRequested != 0) {
				engine_term_display(&engine);
				return;
			}
		}

		if (engine.animating) {
			long currentTime = engine_get_time();
			long deltaTime = currentTime - engine.lastTime;
			engine.lastTime = currentTime;
			
			crAppUpdate(deltaTime / 1000);
			
			// Drawing is throttled to the screen update rate, so there
			// is no need to do timing here.
			engine_draw_frame(&engine);
		}
	}
}
//END_INCLUDE(all)
