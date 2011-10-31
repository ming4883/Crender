//----------------------------------------------------------------------------------
// File:            libs\jni\nv_sound\nv_sound.cpp
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

#include <jni.h>
#include <android/log.h>
#include <stdlib.h>
#include "../nv_thread/nv_thread.h"

#define MODULE "soundlib"
#define THE_ACTIVITY "com/nvidia/devtech/AudioHelper"

#include "nv_sound.h"

static jobject s_globalThiz;
static jclass NvAudioHelper_class;

static jmethodID s_load;
static jmethodID s_loadasset;
static jmethodID s_resume;
static jmethodID s_stop;
static jmethodID s_play;
static jmethodID s_volume;
static jmethodID s_unload;
static jmethodID s_source;
static jmethodID s_startmusic;
static jmethodID s_musicvolume;
static jmethodID s_stopmusic;
static jmethodID s_maxvolume;

void NvSoundInit()
{
	JNIEnv *env = NVThreadGetCurrentJNIEnv();

	NvAudioHelper_class = env->FindClass(THE_ACTIVITY);

	jmethodID getInstance = env->GetStaticMethodID(NvAudioHelper_class, "getInstance", "()Lcom/nvidia/devtech/AudioHelper;");

	__android_log_print(ANDROID_LOG_DEBUG, "apk",  "inst = %d\n", (int)getInstance);

	s_load = env->GetMethodID(NvAudioHelper_class, "LoadSound", "(Ljava/lang/String;I)I");
	s_loadasset = env->GetMethodID(NvAudioHelper_class, "LoadSoundAsset", "(Ljava/lang/String;I)I");
	s_resume = env->GetMethodID(NvAudioHelper_class, "ResumeSound", "(I)V");
	s_stop = env->GetMethodID(NvAudioHelper_class, "StopSound", "(I)V");
	s_play = env->GetMethodID(NvAudioHelper_class, "PlaySound", "(IFFIIF)I");
	s_volume = env->GetMethodID(NvAudioHelper_class, "SetVolume", "(IFF)V");
	s_unload = env->GetMethodID(NvAudioHelper_class, "UnloadSample", "(I)Z");
	s_source = env->GetMethodID(NvAudioHelper_class, "MusicSetDataSource", "(Ljava/lang/String;)V");
	s_startmusic = env->GetMethodID(NvAudioHelper_class, "MusicStart", "()V");
	s_musicvolume = env->GetMethodID(NvAudioHelper_class, "MusicVolume", "(FF)V");
	s_stopmusic = env->GetMethodID(NvAudioHelper_class, "MusicStop", "()V");
	s_maxvolume = env->GetMethodID(NvAudioHelper_class, "SetMaxVolume", "()V");

	jobject thiz = env->CallStaticObjectMethod(NvAudioHelper_class, getInstance);

	if (thiz == NULL)
	{
		__android_log_print(ANDROID_LOG_DEBUG, MODULE, "no this");
	}

	s_globalThiz = env->NewGlobalRef(thiz);
}

void NVSoundShutdown()
{
	s_globalThiz = NULL;
}

int SoundPoolLoadSFX(const char *FileName, int Priority)
{
	int SoundID;

	JNIEnv *env = NVThreadGetCurrentJNIEnv();

	jstring s = env->NewStringUTF(FileName);
	SoundID = env->CallIntMethod(s_globalThiz, s_load, s, Priority);
	env->DeleteLocalRef((jobject)s);

	return SoundID;
}

int SoundPoolLoadSFXAsset(const char *FileName, int Priority)
{
	int SoundID;

	JNIEnv *env = NVThreadGetCurrentJNIEnv();

	jstring s = env->NewStringUTF(FileName);
	SoundID = env->CallIntMethod(s_globalThiz, s_loadasset, s, Priority);
	env->DeleteLocalRef((jobject)s);

	return SoundID;
}

void SoundPoolResume(int StreamID)
{
	JNIEnv *env = NVThreadGetCurrentJNIEnv();

	env->CallVoidMethod(s_globalThiz, s_resume, StreamID);
}

void SoundPoolStop(int StreamID)
{
	JNIEnv *env = NVThreadGetCurrentJNIEnv();

	jint i = StreamID;

	env->CallVoidMethod(s_globalThiz, s_stop, i);
}

int SoundPoolPlaySound(int SoundID, float LeftVolume, float RightVolume, int Priority, int Loop, float Rate)
{
	JNIEnv *env = NVThreadGetCurrentJNIEnv();

	return env->CallIntMethod(s_globalThiz, s_play, SoundID, LeftVolume, RightVolume, Priority, Loop, Rate);
}

void SoundPoolSetVolume(int StreamID, float LeftVolume, float RightVolume)
{
	JNIEnv *env = NVThreadGetCurrentJNIEnv();

	env->CallVoidMethod(s_globalThiz, s_volume, StreamID, LeftVolume, RightVolume);
}

bool SoundPoolUnloadSample(int SoundID)
{
	JNIEnv *env = NVThreadGetCurrentJNIEnv();

	return env->CallBooleanMethod(s_globalThiz, s_unload, SoundID);
}

void MediaPlayerSetDataSource(const char* FileName)
{
	JNIEnv *env = NVThreadGetCurrentJNIEnv();

	jstring s = env->NewStringUTF(FileName);
	env->CallVoidMethod(s_globalThiz, s_source, s);
	env->DeleteLocalRef((jobject)s);
}

void MediaPlayerStart()
{
	JNIEnv *env = NVThreadGetCurrentJNIEnv();

	env->CallVoidMethod(s_globalThiz, s_startmusic);
}

void MediaPlayerSetVolume(float LeftVolume, float RightVolume)
{
	JNIEnv *env = NVThreadGetCurrentJNIEnv();

	env->CallVoidMethod(s_globalThiz, s_musicvolume, LeftVolume, RightVolume);
}

void MediaPlayerStop()
{
	JNIEnv *env = NVThreadGetCurrentJNIEnv();

	env->CallVoidMethod(s_globalThiz, s_stopmusic);
}

void MediaSetMaxVolume()
{
	JNIEnv *env = NVThreadGetCurrentJNIEnv();

	env->CallVoidMethod(s_globalThiz, s_maxvolume);
}
