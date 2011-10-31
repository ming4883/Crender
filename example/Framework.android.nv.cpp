#include <jni.h>
#include <GLES2/gl2.h>

#include <time.h>
#include <android/log.h>

#include <nv_event/nv_event.h>
#include <nv_file/nv_file.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "crframework", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "crframework", __VA_ARGS__))

#include "Framework.h"

CrAppContext crAppContext = {
	"crApp", nullptr
};

void* crOpen(const char* filename)
{
	NvFile* asset = NvFOpen(filename);

	if(nullptr == asset) {
		crDbgStr("failed to open asset '%s'!\n", filename);
	}
	return asset;
}

void crClose(void* handle)
{
	NvFClose((NvFile*)handle);
}

size_t crRead(void* buff, size_t elsize, size_t nelem, void* handle)
{
	return NvFRead(buff, elsize, nelem, (NvFile*)handle);
}

void crSleep(unsigned int milliseconds)
{
}

static long crGetTime() 
{
	struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    return now.tv_sec*1000000 + now.tv_nsec/1000;
}

int32_t NVEventAppInit(int32_t argc, char** argv)
{
	NvFInit();
	LOGI("NVEventAppInit completed");
	return 0;
}

int32_t NVEventAppMain(int32_t argc, char** argv)
{
	bool quit = false;
	bool crInited = false;

	long base_time = crGetTime();
	
	bool uiVisible = false;

	while (!quit) {
		NVEvent ev;
		while (NVEventGetNextEvent(&ev, uiVisible ? 0 : NV_EVENT_WAIT_FOREVER)) {
			switch (ev.m_type) {
			case NV_EVENT_WINDOW_SIZE:
				//s_winWidth = ev.m_data.m_size.m_w;
				//s_winHeight = ev.m_data.m_size.m_h;
				LOGI("Window size event: %d x %d", ev.m_data.m_size.m_w, ev.m_data.m_size.m_h);
				break;
			case NV_EVENT_KEY:
				break;
			case NV_EVENT_CHAR:
				//__android_log_print(ANDROID_LOG_LOGI, MODULE, 
				//	"Char event: 0x%02x", ev.m_data.m_char.m_unichar);
                //keyDownEvent(ev.m_data.m_char.m_unichar);
				break;
			case NV_EVENT_TOUCH:
				//inputEvent((int)(ev.m_data.m_touch.m_x),
				//            (int)(ev.m_data.m_touch.m_y));
				//LOGI("Touch event");
				break;
			case NV_EVENT_RESUME:
			{
				if(!crInited) {
					crAppContext.context = crContextAlloc();
					crAppConfig();
					
					if(CrFalse == crContextInit(crAppContext.context, nullptr)) {
						LOGW("Unable to initialize crContext");
						return -1;
					}
					
					LOGI("crContext initialized");
					crAppInitialize();
					crInited = true;
				}
					
				uiVisible = true;
				break;
			}
			case NV_EVENT_PAUSE:
			{
				crAppFinalize();
				crContextFree(crAppContext.context);
				crInited = false;
	
				uiVisible = false;
				continue;
			}
			case NV_EVENT_QUIT:
				LOGI("Quit event");
				quit = true;
				break;
			default:
				LOGI("UNKNOWN event");
				break;
			};
		}

		long current_time = crGetTime();

		if (uiVisible) {
			long delta_time = current_time - base_time;
			base_time = current_time;
			
			crAppUpdate(delta_time / 1000);
			
			crContextPreRender(crAppContext.context);

			crAppRender();

			crContextPostRender(crAppContext.context);

			crContextSwapBuffers(crAppContext.context);
		}
	}

	return 0;
}

