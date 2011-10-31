//----------------------------------------------------------------------------------
// File:            libs\jni\nv_util\nv_util.cpp
// Samples Version: Android NVIDIA samples 2 
// Email:           tegradev@nvidia.com
// Forum:           http://developer.nvidia.com/tegra/forums/tegra-forums/android-development
//
// Copyright 2009-2010 NVIDIA� Corporation 
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

#include "nv_util.h"
#include <jni.h>
#include "../nv_thread/nv_thread.h"
#include <string.h>

static jmethodID g_hasAppLocalValue;
static jmethodID g_getAppLocalValue;
static jmethodID g_setAppLocalValue;
static jmethodID g_getParameter;
static jobject g_globalThiz;

void NvUtilInit()
{
	JNIEnv *jniEnv = NVThreadGetCurrentJNIEnv();
    jclass k = jniEnv->FindClass("com/nvidia/devtech/NvUtil");
    g_hasAppLocalValue  = jniEnv->GetMethodID(k, "hasAppLocalValue", "(Ljava/lang/String;)Z");
    g_getAppLocalValue  = jniEnv->GetMethodID(k, "getAppLocalValue", "(Ljava/lang/String;)Ljava/lang/String;");
    g_setAppLocalValue  = jniEnv->GetMethodID(k, "setAppLocalValue", "(Ljava/lang/String;Ljava/lang/String;)V");
    g_getParameter      = jniEnv->GetMethodID(k, "getParameter",     "(Ljava/lang/String;)Ljava/lang/String;");

    jmethodID getInstance = jniEnv->GetStaticMethodID(k, "getInstance", "()Lcom/nvidia/devtech/NvUtil;");
    g_globalThiz = jniEnv->CallStaticObjectMethod(k, getInstance);
}

static void NvUtilGetStringValue(jmethodID method, char *buffer, int bufferLength, const char *name)
{
	JNIEnv *jniEnv = NVThreadGetCurrentJNIEnv();
    jstring nameJava = jniEnv->NewStringUTF(name);
    jstring valueJava = (jstring)jniEnv->CallObjectMethod(g_globalThiz, method, nameJava);

    int len = 0;
    if (valueJava)
    {
        const char* string = jniEnv->GetStringUTFChars(valueJava, NULL);

        len = strlen(string);
        if (len > bufferLength-1)
            len = bufferLength-1;
        strncpy(buffer, string, len);
        jniEnv->ReleaseStringUTFChars(valueJava, string);
    }
    buffer[len] = '\0';
}


void NvUtilGetLocalAppValue(char *buffer, int bufferLength, const char* name)
{
    NvUtilGetStringValue(g_getAppLocalValue, buffer, bufferLength, name);
}

bool NvUtilHasLocalAppValue(const char* name)
{
	JNIEnv *jniEnv = NVThreadGetCurrentJNIEnv();
    jstring nameJava = jniEnv->NewStringUTF(name);
    jboolean hasIt = jniEnv->CallBooleanMethod(g_globalThiz, g_hasAppLocalValue, nameJava);

    return (hasIt == JNI_TRUE) ? true : false;
}

void NvUtilSetLocalAppValue(const char* name, const char* value)
{
	JNIEnv *jniEnv = NVThreadGetCurrentJNIEnv();
    jstring nameJava = jniEnv->NewStringUTF(name);
    jstring valueJava = jniEnv->NewStringUTF(value);
    jniEnv->CallVoidMethod(g_globalThiz, g_setAppLocalValue, nameJava, valueJava);
}

void NvUtilGetParameter(char *buffer, int bufferLength, const char *parameter)
{
    NvUtilGetStringValue(g_getParameter, buffer, bufferLength, parameter);
}
