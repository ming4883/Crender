//----------------------------------------------------------------------------------
// File:            libs\jni\nv_event\nv_event.cpp
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

#include <stdlib.h>
#include <jni.h>
#include <pthread.h>
#include <android/log.h>
#include "nv_event.h"
#include "../nv_time/nv_time.h"
#include "../nv_thread/nv_thread.h"
#include <GLES2/gl2.h>
#include <EGL/egl.h>

// TODO TBD - this should be done in NVTimeInit(), but we use a different
// class than most apps.  Need to clean this up, as it is fragile w.r.t.
// changes in nv_time
extern void nvAcquireTimeExtensionJNI(JNIEnv*, jobject);
extern jlong nvGetSystemTimeJNI(JNIEnv*, jobject);

#define MODULE "NVEvent"

#define DBG_DETAILED 0
#include "../nv_debug.h"


#define CT_ASSERT(tag,cond) \
enum { COMPILE_TIME_ASSERT__ ## tag = 1/(cond) }

#define dimof(x) (sizeof(x)/sizeof(x[0]))

#define NV_EVENT_QUEUE_ELEMS 256
#define NV_EVENT_QUEUE_MASK 0x000000ff
#define NV_MAX_KEYCODE 256

inline bool isSacred(NVEventType type)
{
	if( type >= NV_EVENT_WINDOW_SIZE &&
	    type <= NV_EVENT_RESUME )
		return true;
	return false;
}

typedef struct NVEventSync
{
	pthread_cond_t m_cond;
	bool           m_block;
} NVEventSem;

typedef struct NVEventQueue
{
	int32_t         m_nextInsertIndex;
	int32_t         m_headIndex;

	// ***  begin locking related variables ////////////////////////////////////
	// shared mutex used to lock around the condition variable containing     //
	// "sync" objects below and NVEvnet                                       //
	// TODO: Some of this may be separable into multiple mutexes with shorter //
	//       lock times. If we want to have more than 2 threads interacting   //
	//       here, this may become important                                  //
	pthread_mutex_t m_mutex;                                                  //
                                                                              //
	NVEventSync     m_nativeSync;                                             //
	NVEventSync     m_javaSync;                                               //
                                                                              //
	// used to store lists of wait events that the native thread might be     //
	// waiting on. The m_waitEventTypes pointer is also used by the java      //
	// thread to determine if it should scan pushed events to check for the   //
	// need to unblock the native thread                                      //
	const int *     m_waitEventTypes;                                         //
	int			    m_waitEventTypeCount;                                     //
                                                                              //
	// this flag is used by NVEventRemoveOldest to track when to unblock the  //
	// java thread for processing of pause events                             //
	bool            m_processingPause;                                        //
	// ***  end locking related variables //////////////////////////////////////

	NVEvent m_events[NV_EVENT_QUEUE_ELEMS];
} NVEventQueue;

static NVKeyCode s_keyMapping[NV_MAX_KEYCODE];

static NVEventQueue s_eventQueue;


static jobject s_globalThiz;

static jmethodID s_initEGL;
static jmethodID s_swap;
static jmethodID s_makeCurrent;
static jmethodID s_unMakeCurrent;
static jmethodID s_finish;


#ifdef _TIMFUDGE
	static jmethodID s_width;
	static jmethodID s_height;
#endif

static jmethodID s_loadFile;
static jfieldID s_lengthId;
static jfieldID s_dataId;
static jmethodID s_loadTexture;
static jfieldID s_widthId;
static jfieldID s_heightId;
static jfieldID s_texDataId;
static jmethodID s_getOri;

static pthread_t   s_mainThread;

static bool s_alreadyFinishing = false;
static bool s_supportPauseResume = true;  //< This variable is only used for informational purposes in NVEventUnhandledEvent


#define NVNextWrapped(index) (((index) + 1) & NV_EVENT_QUEUE_MASK)
#define NVPrevWrapped(index) (((index) - 1) & NV_EVENT_QUEUE_MASK)

/* you must be inside a m_mutex lock to invoke this! */
static void unlockAll(NVEventSem* sem)
{
	sem->m_block = false;
	pthread_cond_broadcast(&(sem->m_cond));
}

/* you must be inside a m_mutex lock to invoke this! */
static int32_t wait(NVEventSem* sem, pthread_mutex_t* mutex, int waitMS)
{
	if(sem->m_block)
	{
		if( waitMS < 0 )
		{
			return pthread_cond_wait(&sem->m_cond, mutex);
		}
		else
		{
			return pthread_cond_timeout_np(&sem->m_cond, mutex, (unsigned)waitMS);
		}
	}
	return 1;
}

/* you must be inside a m_mutex lock to invoke this! */
static void signal(NVEventSem* sem)
{
	pthread_cond_signal(&sem->m_cond);
}

static void syncInit( NVEventSync* sync )
{
	pthread_cond_init(&(sync->m_cond), NULL);
	sync->m_block = true;
}

static void syncDestroy( NVEventSync* sync )
{
	pthread_cond_destroy( &sync->m_cond );
}

/* Init the mapping array, set up the event queue */
static void AddKeyMapping(JNIEnv* env, jobject thiz, jclass KeyCode_class, const char* name, NVKeyCode value)
{
	// Add a new mapping...
    jfieldID id = env->GetStaticFieldID(KeyCode_class, name, "I");
    int keyID = env->GetStaticIntField(KeyCode_class, id);

	if (keyID < NV_MAX_KEYCODE)
	{
		/* TODO TBD Should check for collision */
		s_keyMapping[keyID] = value;
	}
}

#define AddKeymappingMacro(name, value) \
	AddKeyMapping(env, thiz, KeyCode_class, name, value)

static void initMap(JNIEnv* env, jobject thiz)
{
    jclass KeyCode_class = env->FindClass("android/view/KeyEvent");

	memset(s_keyMapping, 0, sizeof(NVKeyCode) * NV_MAX_KEYCODE);

	AddKeymappingMacro("KEYCODE_BACK",NV_KEYCODE_BACK);
	AddKeymappingMacro("KEYCODE_TAB",NV_KEYCODE_TAB);
	AddKeymappingMacro("KEYCODE_ENTER",NV_KEYCODE_ENTER);

	AddKeymappingMacro("KEYCODE_SPACE",NV_KEYCODE_SPACE);
	AddKeymappingMacro("KEYCODE_ENDCALL",NV_KEYCODE_ENDCALL);
	AddKeymappingMacro("KEYCODE_HOME",NV_KEYCODE_HOME);

	AddKeymappingMacro("KEYCODE_DPAD_LEFT",NV_KEYCODE_DPAD_LEFT);
	AddKeymappingMacro("KEYCODE_DPAD_UP",NV_KEYCODE_DPAD_UP);
	AddKeymappingMacro("KEYCODE_DPAD_RIGHT",NV_KEYCODE_DPAD_RIGHT);
	AddKeymappingMacro("KEYCODE_DPAD_DOWN",NV_KEYCODE_DPAD_DOWN);

	AddKeymappingMacro("KEYCODE_DEL",NV_KEYCODE_DEL);

	AddKeymappingMacro("KEYCODE_0",NV_KEYCODE_0);
	AddKeymappingMacro("KEYCODE_1",NV_KEYCODE_1);
	AddKeymappingMacro("KEYCODE_2",NV_KEYCODE_2);
	AddKeymappingMacro("KEYCODE_3",NV_KEYCODE_3);
	AddKeymappingMacro("KEYCODE_4",NV_KEYCODE_4);
	AddKeymappingMacro("KEYCODE_5",NV_KEYCODE_5);
	AddKeymappingMacro("KEYCODE_6",NV_KEYCODE_6);
	AddKeymappingMacro("KEYCODE_7",NV_KEYCODE_7);
	AddKeymappingMacro("KEYCODE_8",NV_KEYCODE_8);
	AddKeymappingMacro("KEYCODE_9",NV_KEYCODE_9);

	AddKeymappingMacro("KEYCODE_A",NV_KEYCODE_A);
	AddKeymappingMacro("KEYCODE_B",NV_KEYCODE_B);
	AddKeymappingMacro("KEYCODE_C",NV_KEYCODE_C);
	AddKeymappingMacro("KEYCODE_D",NV_KEYCODE_D);
	AddKeymappingMacro("KEYCODE_E",NV_KEYCODE_E);
	AddKeymappingMacro("KEYCODE_F",NV_KEYCODE_F);
	AddKeymappingMacro("KEYCODE_G",NV_KEYCODE_G);
	AddKeymappingMacro("KEYCODE_H",NV_KEYCODE_H);
	AddKeymappingMacro("KEYCODE_I",NV_KEYCODE_I);
	AddKeymappingMacro("KEYCODE_J",NV_KEYCODE_J);
	AddKeymappingMacro("KEYCODE_K",NV_KEYCODE_K);
	AddKeymappingMacro("KEYCODE_L",NV_KEYCODE_L);
	AddKeymappingMacro("KEYCODE_M",NV_KEYCODE_M);
	AddKeymappingMacro("KEYCODE_N",NV_KEYCODE_N);
	AddKeymappingMacro("KEYCODE_O",NV_KEYCODE_O);
	AddKeymappingMacro("KEYCODE_P",NV_KEYCODE_P);
	AddKeymappingMacro("KEYCODE_Q",NV_KEYCODE_Q);
	AddKeymappingMacro("KEYCODE_R",NV_KEYCODE_R);
	AddKeymappingMacro("KEYCODE_S",NV_KEYCODE_S);
	AddKeymappingMacro("KEYCODE_T",NV_KEYCODE_T);
	AddKeymappingMacro("KEYCODE_U",NV_KEYCODE_U);
	AddKeymappingMacro("KEYCODE_V",NV_KEYCODE_V);
	AddKeymappingMacro("KEYCODE_W",NV_KEYCODE_W);
	AddKeymappingMacro("KEYCODE_X",NV_KEYCODE_X);
	AddKeymappingMacro("KEYCODE_Y",NV_KEYCODE_Y);
	AddKeymappingMacro("KEYCODE_Z",NV_KEYCODE_Z);

	AddKeymappingMacro("KEYCODE_STAR",NV_KEYCODE_STAR);
	AddKeymappingMacro("KEYCODE_PLUS",NV_KEYCODE_PLUS);
	AddKeymappingMacro("KEYCODE_MINUS",NV_KEYCODE_MINUS);

	AddKeymappingMacro("KEYCODE_NUM",NV_KEYCODE_NUM);

	AddKeymappingMacro("KEYCODE_ALT_LEFT",NV_KEYCODE_ALT_LEFT);
	AddKeymappingMacro("KEYCODE_ALT_RIGHT",NV_KEYCODE_ALT_RIGHT);

	AddKeymappingMacro("KEYCODE_SHIFT_LEFT",NV_KEYCODE_SHIFT_LEFT);
	AddKeymappingMacro("KEYCODE_SHIFT_RIGHT",NV_KEYCODE_SHIFT_RIGHT);

	AddKeymappingMacro("KEYCODE_APOSTROPHE",NV_KEYCODE_APOSTROPHE);
	AddKeymappingMacro("KEYCODE_SEMICOLON",NV_KEYCODE_SEMICOLON);
	AddKeymappingMacro("KEYCODE_EQUALS",NV_KEYCODE_EQUALS);
	AddKeymappingMacro("KEYCODE_COMMA",NV_KEYCODE_COMMA);
	AddKeymappingMacro("KEYCODE_PERIOD",NV_KEYCODE_PERIOD);
	AddKeymappingMacro("KEYCODE_SLASH",NV_KEYCODE_SLASH);
	AddKeymappingMacro("KEYCODE_GRAVE",NV_KEYCODE_GRAVE);
	AddKeymappingMacro("KEYCODE_LEFT_BRACKET",NV_KEYCODE_LEFT_BRACKET);
	AddKeymappingMacro("KEYCODE_BACKSLASH",NV_KEYCODE_BACKSLASH);
	AddKeymappingMacro("KEYCODE_RIGHT_BRACKET",NV_KEYCODE_RIGHT_BRACKET);

}

const char* NVEventGetEventStr(NVEventType eventType)
{
	switch(eventType)
	{
		case NV_EVENT_KEY:         return "NV_EVENT_KEY";
		case NV_EVENT_CHAR:        return "NV_EVENT_CHAR";
		case NV_EVENT_TOUCH:       return "NV_EVENT_TOUCH";
		case NV_EVENT_MULTITOUCH:  return "NV_EVENT_MULTITOUCH";
		case NV_EVENT_ACCEL:       return "NV_EVENT_ACCEL";
		case NV_EVENT_WINDOW_SIZE: return "NV_EVENT_WINDOW_SIZE";
		case NV_EVENT_QUIT:        return "NV_EVENT_QUIT";
		case NV_EVENT_PAUSE:       return "NV_EVENT_PAUSE";
		case NV_EVENT_RESUME:      return "NV_EVENT_RESUME";
	}
	// update this if you end up having to edit something.
	CT_ASSERT(NEED_TO_ADD_STRING_HERE, NV_EVENT_NUM_EVENTS == 9);
	return "unknown event type!";
}

int NVEventGetOrientation()
{
	JNIEnv* env = NVThreadGetCurrentJNIEnv();
	return env->CallIntMethod(s_globalThiz, s_getOri);
}

char* NVEventLoadFile(const char* file)
{
	JNIEnv* env = NVThreadGetCurrentJNIEnv();

	jstring test = env->NewStringUTF(file);
    jobject rawData = env->CallObjectMethod(s_globalThiz, s_loadFile, test);

    jbyteArray data = (jbyteArray) env->GetObjectField(rawData, s_dataId);
    int size = env->GetIntField(rawData, s_lengthId);

    unsigned char* data2 = (unsigned char*) env->GetByteArrayElements(data, NULL);
	char * buffer = new char[size+1];
	memcpy(buffer, data2, size);
	buffer[size] = '\0';

	env->ReleaseByteArrayElements(data, (jbyte*) data2, 0);
    env->DeleteLocalRef(rawData);

	return buffer;
}

void* NVEventGetTextureData(const char* filename, unsigned char*& pixels, 
							unsigned int& width, unsigned int& height,
							unsigned int& format, unsigned int& type)
{
	JNIEnv* env = NVThreadGetCurrentJNIEnv();

    jstring test = env->NewStringUTF(filename);
    jobject rawTexture = env->CallObjectMethod(s_globalThiz, s_loadTexture, test);

    jbyteArray data = (jbyteArray) env->GetObjectField(rawTexture, s_texDataId);
    width = env->GetIntField(rawTexture, s_widthId);
    height = env->GetIntField(rawTexture, s_heightId);

    format = GL_RGBA;
	type = GL_UNSIGNED_BYTE;

    pixels = (unsigned char*) env->GetByteArrayElements(data, NULL);
    env->DeleteLocalRef(rawTexture);

	return (void*)data;
}

void NVEventReleaseTextureData(void* data, unsigned char* pixels)
{
	JNIEnv* env = NVThreadGetCurrentJNIEnv();

	env->ReleaseByteArrayElements((jbyteArray)data, (jbyte*)pixels, JNI_ABORT);
	env->DeleteLocalRef((jbyteArray)data);
}

static void NVEventInit(JNIEnv* env, jobject thiz)
{
    if (!s_globalThiz)
    {
        s_globalThiz = env->NewGlobalRef(thiz);
        if (!s_globalThiz)
        {
            __android_log_print(ANDROID_LOG_DEBUG, MODULE,  "Error: Thiz NewGlobalRef failed!");
        }

        __android_log_print(ANDROID_LOG_DEBUG, MODULE,  "Thiz NewGlobalRef: 0x%p", s_globalThiz);
    }

    __android_log_print(ANDROID_LOG_DEBUG, MODULE,  "initMap");
	initMap(env, thiz);

	__android_log_print(ANDROID_LOG_DEBUG, MODULE,  "file methods");
	
    jclass activity_class = env->FindClass("com/nvidia/devtech/NvEventQueueActivity");
    s_loadFile = env->GetMethodID(activity_class, "loadFile", "(Ljava/lang/String;)Lcom/nvidia/devtech/NvEventQueueActivity$RawData;");
    jclass RawData_class = env->FindClass("com/nvidia/devtech/NvEventQueueActivity$RawData");
    s_lengthId    = env->GetFieldID(RawData_class, "length", "I");
    s_dataId      = env->GetFieldID(RawData_class, "data", "[B");

	__android_log_print(ANDROID_LOG_DEBUG, MODULE,  "texture methods");
	
    s_loadTexture = env->GetMethodID(activity_class, "loadTexture", "(Ljava/lang/String;)Lcom/nvidia/devtech/NvEventQueueActivity$RawTexture;");
    jclass RawTexture_class = env->FindClass("com/nvidia/devtech/NvEventQueueActivity$RawTexture");
    s_widthId    = env->GetFieldID(RawTexture_class, "width", "I");
    s_heightId   = env->GetFieldID(RawTexture_class, "height", "I");
    s_texDataId     = env->GetFieldID(RawTexture_class, "data", "[B");

	s_getOri = env->GetMethodID(activity_class, "getOrientation", "()I");

	s_eventQueue.m_nextInsertIndex = 0;
	s_eventQueue.m_headIndex = 0;
	pthread_mutex_init(&(s_eventQueue.m_mutex), NULL);
	syncInit(&s_eventQueue.m_nativeSync);
	syncInit(&s_eventQueue.m_javaSync);
	s_eventQueue.m_processingPause = false;
	s_eventQueue.m_waitEventTypeCount = 0;
	s_eventQueue.m_waitEventTypes = NULL;
	__android_log_print(ANDROID_LOG_DEBUG, MODULE,  "Leave NVEventInit");
}

static void NVEventShutdown()
{
    JNIEnv* jniEnv = NVThreadGetCurrentJNIEnv();

    if (!jniEnv || !s_globalThiz)
    {
        __android_log_print(ANDROID_LOG_DEBUG, MODULE,  "Error: DestroyingRegisteredObjectInstance no TLS data!");
    }

    jniEnv->DeleteGlobalRef(s_globalThiz);
	s_globalThiz = NULL;

    __android_log_print(ANDROID_LOG_DEBUG, MODULE,  "Released global thiz!");

	pthread_mutex_destroy(&(s_eventQueue.m_mutex));

	// free everyone...
	unlockAll(&s_eventQueue.m_nativeSync);
	unlockAll(&s_eventQueue.m_javaSync);
	syncDestroy(&(s_eventQueue.m_nativeSync));
	syncDestroy(&(s_eventQueue.m_javaSync));
}

/* you must be inside a m_mutex lock to invoke this! */
static bool isEventType(NVEventQueue* q, const NVEvent* event)
{
	for(int32_t i = 0; i < q->m_waitEventTypeCount; ++i)
	{
		if(event->m_type == q->m_waitEventTypes[i])
			return true;
	}
	return false;
}

/* you must be inside a m_mutex lock to invoke this! */
static bool scanForEvents(NVEventQueue* q)
{
	DEBUG("scanForEvents");
	for(int32_t i = 0; i < q->m_waitEventTypeCount; ++i)
		DEBUG("    event %s", NVEventGetEventStr((NVEventType)q->m_waitEventTypes[i]));

	// scan events in our queue, return true if found and
	// set ev if it's not null
	for(int32_t i = q->m_headIndex; i != q->m_nextInsertIndex; i = NVNextWrapped(i))
	{
		const NVEvent* event = &q->m_events[i];
		DEBUG("examining event type [%d]: %s", i, NVEventGetEventStr(event->m_type));
		if (isEventType(q, event))
		{
			DEBUG("event matched");
			return true;
		}
	}
	DEBUG("event not matched");
	return false;
}


/* you must be inside a m_mutex lock to invoke this! */
static void insert(NVEventQueue* q, NVEvent* ev)
{
	// Is the queue full?
	int32_t nextNext = NVNextWrapped(q->m_nextInsertIndex);
	if (nextNext == q->m_headIndex)
	{
		int32_t cur = nextNext;
		while( (cur != q->m_nextInsertIndex) &&
		       isSacred(q->m_events[cur].m_type) )
		{
			cur = NVNextWrapped(cur);
		}

		// if we didn't go all the way around, replace the cur and return
		if( cur != q->m_nextInsertIndex )
		{
			memcpy(q->m_events + cur, ev, sizeof(NVEvent));
			__android_log_print(ANDROID_LOG_DEBUG, MODULE, "FULL QUEUE");
			return;
		}
	}

	memcpy(q->m_events + q->m_nextInsertIndex, ev, sizeof(NVEvent));
	q->m_nextInsertIndex = nextNext;
}

static void NVEventFlushQueue(NVEventQueue* q)
{
	pthread_mutex_lock(&(q->m_mutex));

	int newNextInsertIndex = q->m_headIndex;
	for(int i = q->m_headIndex; i != q->m_nextInsertIndex; i = NVNextWrapped(i))
	{
		NVEvent* ev = &(q->m_events[i]);
		if(isSacred(ev->m_type))
		{
			q->m_events[newNextInsertIndex++] = *ev;
			break;
		}
	}

	pthread_mutex_unlock(&(q->m_mutex));
}

// if a pause event is inserted, signal the event inserted m_nativeSync
// and block on the java m_javaSync, this allows the java thread
// to be unblocked after the native thread indicates that it is done
// processing the pause event -- by pulling the next event from the queue
static void NVEventInsertNewest(NVEventQueue* q, NVEvent* ev)
{
	if(!s_alreadyFinishing)
	{
		pthread_mutex_lock(&(q->m_mutex));

		// if we are waiting for a set of event types, check against those
		if (q->m_waitEventTypes != 0)
		{
			if (!isEventType(q, ev))
			{
				// the event isn't one of interest
				// insert it and unlock the mutex. Don't signal the cond.
				insert(q, ev);
				pthread_mutex_unlock(&(q->m_mutex));
				return;
			}
		}
		else if(ev->m_type == NV_EVENT_PAUSE)
		{
			// processing a pause, insert the event, unblock the queue waiter
			// and wait for the pause to be processed
			insert(q, ev);
			signal(&(q->m_nativeSync));
			wait(&(q->m_javaSync), &(q->m_mutex), NV_EVENT_WAIT_FOREVER);
			DEBUG("java unblocking from native handling pause");
			pthread_mutex_unlock(&(q->m_mutex));
			return;
		}

		// insert the event and unblock a waiter
		insert(q, ev);
		signal(&(q->m_nativeSync));
		pthread_mutex_unlock(&(q->m_mutex));
	}
}

static int32_t NVEventRemoveOldest(NVEventQueue* q, NVEvent* ev, int waitMSecs)
{
	pthread_mutex_lock(&(q->m_mutex));
	int retVal = 1;
	if(q->m_processingPause)
	{
		q->m_processingPause = false;
		signal(&(q->m_javaSync));
	}

	if (q->m_nextInsertIndex == q->m_headIndex)
	{
		// We're empty - so what do we do?
		if (waitMSecs == 0)
		{
			// can't wait - gotta go!  Return no event;
			retVal = 0;
			goto no_event;
		}
		else
		{
			// wait for the specified time
			wait(&(q->m_nativeSync), &(q->m_mutex), (unsigned)waitMSecs);
		}

		// check again after exiting cond waits, either we had a timeout
		if (q->m_nextInsertIndex == q->m_headIndex)
		{
			// Still nothing
			retVal = 0;
			goto no_event;
		}
	}

	// One way or another, we have an event...
	memcpy(ev, q->m_events + q->m_headIndex, sizeof(NVEvent));
	if(ev->m_type == NV_EVENT_PAUSE)
		q->m_processingPause = true;
	q->m_headIndex = NVNextWrapped(q->m_headIndex);

no_event:
	pthread_mutex_unlock(&(q->m_mutex));
	return retVal;
}

/* Get next event */
int32_t NVEventGetNextEvent(NVEvent* ev, int waitMSecs)
{
	return NVEventRemoveOldest(&s_eventQueue, ev, waitMSecs);
}

int32_t NVEventWaitForEvents(const int* eventTypes, int numEvents, int waitMSecs)
{
	int32_t ret = 0;
	pthread_mutex_lock(&(s_eventQueue.m_mutex));
	DEBUG("entered NVEventWaitForEvents, wait time %d", waitMSecs);

	s_eventQueue.m_waitEventTypes = eventTypes;
	s_eventQueue.m_waitEventTypeCount = numEvents;

	if (scanForEvents(&s_eventQueue))
	{
		DEBUG("");
		ret=1;
	}
	else
	{
		wait(&s_eventQueue.m_nativeSync, &(s_eventQueue.m_mutex), waitMSecs);
		if (scanForEvents(&s_eventQueue))
			ret = 1;
	}

	s_eventQueue.m_waitEventTypes = 0;
	s_eventQueue.m_waitEventTypeCount = 0;

	pthread_mutex_unlock(&(s_eventQueue.m_mutex));
	DEBUG("exited NVEventWaitForEvents");
	return ret;
}

void NVEventUnhandledEvent(NVEvent* ev)
{
	if (s_supportPauseResume && (ev->m_type == NV_EVENT_PAUSE || ev->m_type == NV_EVENT_QUIT))
	{
		DEBUG_D("\"supportPauseResume\" applications should handle NV_EVENT_PAUSE and NV_EVENT_QUIT");
	}
}

bool NVEventEGLInit()
{
    JNIEnv* jniEnv = NVThreadGetCurrentJNIEnv();

    if (!jniEnv || !s_globalThiz)
    {
        __android_log_print(ANDROID_LOG_DEBUG, MODULE,  "Error: No valid JNI env in MakeCurrent");
		return false;
    }
	else if (!jniEnv->CallBooleanMethod(s_globalThiz, s_initEGL))
	{
		return false;
	}
	else
	{
		return true;
	}
}

void NVEventEGLMakeCurrent()
{
    JNIEnv* jniEnv = NVThreadGetCurrentJNIEnv();

    if (!jniEnv || !s_globalThiz)
    {
        __android_log_print(ANDROID_LOG_DEBUG, MODULE,  "Error: No valid JNI env in MakeCurrent");
    }
	else if (!jniEnv->CallBooleanMethod(s_globalThiz, s_makeCurrent))
	{
        __android_log_print(ANDROID_LOG_DEBUG, MODULE,  "Error: MakeCurrent failed");
	}
}

void NVEventEGLUnmakeCurrent()
{
    JNIEnv* jniEnv = NVThreadGetCurrentJNIEnv();

    if (!jniEnv || !s_globalThiz)
    {
        __android_log_print(ANDROID_LOG_DEBUG, MODULE,  "Error: No valid JNI env in UnMakeCurrent");
    }
	else if (!jniEnv->CallBooleanMethod(s_globalThiz, s_unMakeCurrent))
	{
        __android_log_print(ANDROID_LOG_DEBUG, MODULE,  "Error: UnMakeCurrent failed");
	}
}

#define PERF_STMTS 0
#if PERF_STMTS == 1
	#include "stdio.h"
	static char s_bigString[4096];
	static int s_bigStringSize;

	static char s_tmpBuf[1024];

	class ScopedProfiler
	{
	public:
		ScopedProfiler(const char* text)
		{
			_text = text;
			_startTime = nvGetSystemTime();
			__last = this;
		}
		~ScopedProfiler()
		{
			stop();
		}
		inline void stop()
		{
			if(_text)
			{
				int size = snprintf(s_tmpBuf, dimof(s_tmpBuf)-1, "%d ms spent in %s" , (int)(nvGetSystemTime() - _startTime), _text);
				strcat(s_bigString + s_bigStringSize, s_tmpBuf);
				s_bigStringSize += size;
				_text = 0;
			}
		}
		static void stopLast()
		{
			if(__last)
				__last->stop();
			__last = 0;
		}
		const char* _text;
		long _startTime;
		static ScopedProfiler* __last;
	};
	ScopedProfiler* ScopedProfiler::__last = 0;
		
	#define STRINGIFIER(s) #s
	#define CONCAT_(a,b) a ## b
	#define CONCAT(a,b) CONCAT_(a,b)
	#define PERFBLURB(s) static const char CONCAT(___str,__LINE__)[] = s "\n"; ScopedProfiler CONCAT(SCOPED_PROFILER,__LINE__)(CONCAT(___str,__LINE__));
	#define RESET_PROFILING()  {  DEBUG_D("%s", s_bigString);  s_bigString[0] = 0;  s_bigStringSize = 0;  }
#else
	#define PERFBLURB(s)
	#define RESET_PROFILING()
#endif

void NVEventEGLSwapBuffers()
{
    JNIEnv* jniEnv = NVThreadGetCurrentJNIEnv();

	PERFBLURB("swap JNI");
    if (!jniEnv || !s_globalThiz)
    {
        __android_log_print(ANDROID_LOG_DEBUG, MODULE,  "Error: No valid JNI env in SwapBuffers");
    }
	else if (!jniEnv->CallBooleanMethod(s_globalThiz, s_swap))
	{
        __android_log_print(ANDROID_LOG_DEBUG, MODULE,  "Error: SwapBuffers failed");
	}
	RESET_PROFILING();
}

#ifdef _TIMFUDGE
int NVEventEGLGetSurfaceWidth()
{
    JNIEnv* jniEnv = NVThreadGetCurrentJNIEnv();

    if (!jniEnv || !s_globalThiz)
    {
        __android_log_print(ANDROID_LOG_DEBUG, MODULE,  "Error: No valid JNI env in NVEventEGLGetSurfaceWidth");

        return 0;
    }

    return jniEnv->CallIntMethod(s_globalThiz, s_width);
}

int NVEventEGLGetSurfaceHeight()
{
    JNIEnv* jniEnv = NVThreadGetCurrentJNIEnv();

    if (!jniEnv || !s_globalThiz)
    {
        __android_log_print(ANDROID_LOG_DEBUG, MODULE,  "Error: No valid JNI env in NVEventEGLGetSurfaceHeight");

        return 0;
    }

	return jniEnv->CallIntMethod(s_globalThiz, s_height);

}
#endif

static void* NVEventMainLoopThreadFunc(void*)
{
	NVEventAppMain(0, NULL);
    __android_log_print(ANDROID_LOG_DEBUG, MODULE,  "NvEvent native app Main returned");

	// signal the condition variable to unblock
	// java from waiting on pause or quit
	unlockAll(&(s_eventQueue.m_javaSync));

    s_alreadyFinishing = true;

	JNIEnv* env = NVThreadGetCurrentJNIEnv(); 
    env->CallVoidMethod(s_globalThiz, s_finish);

	return NULL;
}

static jboolean NVEventJNIInit(JNIEnv*  env, jobject  thiz)
{
	NVEventInit(env, thiz);

	__android_log_print(ANDROID_LOG_DEBUG, MODULE,  "Calling NVEventAppInit");

	if (NVEventAppInit(0, NULL))
	{
		__android_log_print(ANDROID_LOG_DEBUG, MODULE,  "NVEventAppInit error");
		return JNI_FALSE;
	}

    jclass k = (env)->FindClass ("com/nvidia/devtech/NvEventQueueActivity");
	jmethodID id = env->GetMethodID(k, "getSupportPauseResume", "()Z");
	if(id)
	{
		s_supportPauseResume = env->CallBooleanMethod(s_globalThiz, id);
	}

	__android_log_print(ANDROID_LOG_DEBUG, MODULE,  "spawning thread");

    s_alreadyFinishing = false;

    /* Launch thread with main loop */
	NVThreadSpawnJNIThread(&s_mainThread, NULL, NVEventMainLoopThreadFunc, NULL);

	__android_log_print(ANDROID_LOG_DEBUG, MODULE,  "thread spawned");

	return JNI_TRUE;
}

static void NVEventCleanup(JNIEnv*  env)
{
	NVEventShutdown();
}

static jboolean NVEventTouchEvent(JNIEnv*  env, jobject  thiz, jint action, jint mx, jint my)
{
    static jclass KeyCode_class = env->FindClass("android/view/MotionEvent");
    static jfieldID ACTION_DOWN_id = env->GetStaticFieldID(KeyCode_class, "ACTION_DOWN", "I");
    static jfieldID ACTION_UP_id = env->GetStaticFieldID(KeyCode_class, "ACTION_UP", "I");
    static int ACTION_DOWN = env->GetStaticIntField(KeyCode_class, ACTION_DOWN_id);
    static int ACTION_UP = env->GetStaticIntField(KeyCode_class, ACTION_UP_id);

	{
		NVEvent ev;
		ev.m_type = NV_EVENT_TOUCH;
		ev.m_data.m_touch.m_action = (ACTION_UP == action) 
			? NV_TOUCHACTION_UP : (
			(ACTION_DOWN == action) ? NV_TOUCHACTION_DOWN : NV_TOUCHACTION_MOVE);
		ev.m_data.m_touch.m_x = mx;
		ev.m_data.m_touch.m_y = my;
		NVEventInsertNewest(&s_eventQueue, &ev);
	}

	return JNI_TRUE;
}

static jboolean NVEventMultiTouchEvent(JNIEnv*  env, jobject  thiz, jint action, 
									   jint count, jint mx1, jint my1, jint mx2, jint my2)
{
    static jclass KeyCode_class = env->FindClass("android/view/MotionEvent");
    static jfieldID ACTION_DOWN_id = env->GetStaticFieldID(KeyCode_class, "ACTION_DOWN", "I");
    static jfieldID ACTION_UP_id = env->GetStaticFieldID(KeyCode_class, "ACTION_UP", "I");
    static jfieldID ACTION_CANCEL_id = env->GetStaticFieldID(KeyCode_class, "ACTION_CANCEL", "I");
    static jfieldID ACTION_POINTER_INDEX_SHIFT_id = env->GetStaticFieldID(KeyCode_class, "ACTION_POINTER_ID_SHIFT", "I");
    static jfieldID ACTION_POINTER_INDEX_MASK_id = env->GetStaticFieldID(KeyCode_class, "ACTION_POINTER_ID_MASK", "I");
    static int ACTION_DOWN = env->GetStaticIntField(KeyCode_class, ACTION_DOWN_id);
    static int ACTION_UP = env->GetStaticIntField(KeyCode_class, ACTION_UP_id);
    static int ACTION_CANCEL = env->GetStaticIntField(KeyCode_class, ACTION_CANCEL_id);
    static int ACTION_POINTER_INDEX_MASK = env->GetStaticIntField(KeyCode_class, ACTION_POINTER_INDEX_MASK_id);
    static int ACTION_POINTER_INDEX_SHIFT = env->GetStaticIntField(KeyCode_class, ACTION_POINTER_INDEX_SHIFT_id);

	{
		NVEvent ev;

		int actionOnly = action & (~ACTION_POINTER_INDEX_MASK);
        int maskOnly = (count>=2) ? 0x3 : ((count==0) ? 0x0 : 0x1);

		ev.m_type = NV_EVENT_MULTITOUCH;
		
		if (actionOnly == ACTION_UP)
		{
			ev.m_data.m_multi.m_action = NV_MULTITOUCH_UP;
		}
		else if (actionOnly == ACTION_DOWN)
		{
			ev.m_data.m_multi.m_action = NV_MULTITOUCH_DOWN;
		}
		else if (actionOnly == ACTION_CANCEL)
		{
			ev.m_data.m_multi.m_action = NV_MULTITOUCH_CANCEL;
		}
		else
		{
			ev.m_data.m_multi.m_action = NV_MULTITOUCH_MOVE;
		}
		ev.m_data.m_multi.m_action = 
			(NVMultiTouchEventType)(ev.m_data.m_multi.m_action | (maskOnly << NV_MULTITOUCH_POINTER_SHIFT));
		ev.m_data.m_multi.m_x1 = mx1;
		ev.m_data.m_multi.m_y1 = my1;
		ev.m_data.m_multi.m_x2 = mx2;
		ev.m_data.m_multi.m_y2 = my2;
		NVEventInsertNewest(&s_eventQueue, &ev);
	}

	return JNI_TRUE;
}

static jboolean NVEventKeyEvent(JNIEnv* env, jobject thiz, jint action, jint keycode, jint unichar)
{
    static jclass KeyCode_class = env->FindClass("android/view/KeyEvent");
    static jfieldID ACTION_UP_id = env->GetStaticFieldID(KeyCode_class, "ACTION_UP", "I");
    static int ACTION_UP = env->GetStaticIntField(KeyCode_class, ACTION_UP_id);
	static int lastKeyAction = 0;
	static int lastKeyCode = 0;

	if (keycode < NV_MAX_KEYCODE)
	{
		NVKeyCode code = s_keyMapping[keycode];

		if ((code != NV_KEYCODE_NULL) &&
			((code != lastKeyCode) || (action != lastKeyAction)))
		{
			NVEvent ev;
			ev.m_type = NV_EVENT_KEY;
			ev.m_data.m_key.m_action = (ACTION_UP == action) 
				? NV_KEYACTION_UP : NV_KEYACTION_DOWN;
			ev.m_data.m_key.m_code = code;
			NVEventInsertNewest(&s_eventQueue, &ev);
		}

		lastKeyAction = action;
		lastKeyCode = code;
	}


	if (unichar && (ACTION_UP != action))
	{
		NVEvent ev;
		ev.m_type = NV_EVENT_CHAR;
		ev.m_data.m_char.m_unichar = unichar;
		NVEventInsertNewest(&s_eventQueue, &ev);
	}

    return JNI_TRUE;
}

static jboolean NVEventAccelerometerEvent(JNIEnv* env, jobject thiz, jfloat values0, jfloat values1, jfloat values2)
{
	NVEvent ev;
	ev.m_type = NV_EVENT_ACCEL;
	ev.m_data.m_accel.m_x = values0;
	ev.m_data.m_accel.m_y = values1;
	ev.m_data.m_accel.m_z = values2;
	NVEventInsertNewest(&s_eventQueue, &ev);
	return JNI_TRUE;
}

static void NVEventSetWindowSize(JNIEnv* env, jobject thiz, int w, int h)
{
	NVEvent ev;
	ev.m_type = NV_EVENT_WINDOW_SIZE;
	ev.m_data.m_size.m_w = w;
	ev.m_data.m_size.m_h = h;
	NVEventInsertNewest(&s_eventQueue, &ev);
}

static void NVEventQuitAndWait()
{
    if (!s_alreadyFinishing)
    {
	    __android_log_print(ANDROID_LOG_DEBUG, MODULE, "Posting quit event");

		// flush ALL events
		s_eventQueue.m_headIndex = s_eventQueue.m_nextInsertIndex;

	    NVEvent ev;
	    ev.m_type = NV_EVENT_QUIT;
	    NVEventInsertNewest(&s_eventQueue, &ev);

		// ensure that the native side
		// isn't blocked waiting for an event -- since we've flushed
		// all the events save quit, we must artificially unblock native
		unlockAll(&(s_eventQueue.m_nativeSync));

	    __android_log_print(ANDROID_LOG_DEBUG, MODULE, "Waiting for main loop exit");
	    pthread_join(s_mainThread, NULL);
	    __android_log_print(ANDROID_LOG_DEBUG, MODULE, "Main loop exited");
    }
}

static void NVEventPostCleanup()
{
    // Stub for now
}


static void NVEventResumeEvent()
{
	NVEventFlushQueue( &s_eventQueue );

	NVEvent ev;
	ev.m_type = NV_EVENT_RESUME;
	NVEventInsertNewest(&s_eventQueue, &ev);
}

static void NVEventPauseEvent()
{
	// TODO: we could selectively flush here to
	//       improve responsiveness to the pause

	NVEventFlushQueue( &s_eventQueue);
	NVEvent ev;
	ev.m_type = NV_EVENT_PAUSE;
	NVEventInsertNewest(&s_eventQueue, &ev);
}

NVEventPlatformAppHandle NVEventGetPlatformAppHandle()
{
	return s_globalThiz;
}

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv *env;

	NVThreadInit(vm);
 
    DEBUG("JNI_OnLoad called");
    if (vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
    {
    	DEBUG("Failed to get the environment using GetEnv()");
        return -1;
    }
    JNINativeMethod methods[] =
    {
		{
            "init",
            "()Z",
            (void *) NVEventJNIInit
        },
        {
			"touchEvent",
			"(IIILandroid/view/MotionEvent;)Z",
			(void *) NVEventTouchEvent

        },
        {
			"multiTouchEvent",
			"(IIIIIILandroid/view/MotionEvent;)Z",
			(void *) NVEventMultiTouchEvent

        },
        {
            "keyEvent",
            "(IIILandroid/view/KeyEvent;)Z",
            (void *) NVEventKeyEvent
        },
        {
            "accelerometerEvent",
            "(FFF)Z",
            (void *) NVEventAccelerometerEvent
        },
        {
            "cleanup",
            "()V",
            (void *) NVEventCleanup
        },
		// TODO TBD - this should be done in NVTimeInit(), but we use a different
		// class than most apps.  Need to clean this up, as it is fragile w.r.t.
		// changes in nv_time
        {
            "nvAcquireTimeExtension",
            "()V",
            (void *) nvAcquireTimeExtensionJNI
        },
        {
            "nvGetSystemTime",
            "()J",
            (void *) nvGetSystemTimeJNI
        },
		{
			"setWindowSize",
			"(II)V",
			(void *) NVEventSetWindowSize
		},
        {
            "quitAndWait",
            "()V",
            (void *) NVEventQuitAndWait
        },
        {
            "postCleanup",
            "()V",
            (void *) NVEventPostCleanup
        },
		{
			"resumeEvent",
			"()V",
			(void *) NVEventResumeEvent
		},
		{
			"pauseEvent",
			"()V",
			(void *) NVEventPauseEvent
		},
    };
    jclass k;
    k = (env)->FindClass ("com/nvidia/devtech/NvEventQueueActivity");
    (env)->RegisterNatives(k, methods, dimof(methods));
	
    s_initEGL = env->GetMethodID(k, "InitEGLAndGLES2", "()Z");
    s_finish = env->GetMethodID(k, "finish", "()V");
    s_swap = env->GetMethodID(k, "swapBuffers", "()Z");
    s_makeCurrent = env->GetMethodID(k, "makeCurrent", "()Z");
    s_unMakeCurrent = env->GetMethodID(k, "unMakeCurrent", "()Z");

#ifdef _TIMFUDGE
    s_width = env->GetMethodID(k, "getSurfaceWidth", "()I");
    s_height = env->GetMethodID(k, "getSurfaceHeight", "()I");
#endif

    return JNI_VERSION_1_4;
}

