//----------------------------------------------------------------------------------
// File:            libs\jni\nv_apk_file\nv_apk_file.c
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

#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <jni.h>
#include <android/log.h>
#include <stdio.h>

#include "nv_apk_file.h"
#include "../nv_thread/nv_thread.h"

static jobject s_globalThiz;
static jclass NvAPKFileClass;
static jclass fileHelper;
static jmethodID s_openFile;
static jmethodID s_closeFile;
static jfieldID s_lengthId;
static jmethodID s_seekFile;
static jfieldID s_positionId;
static jmethodID s_readFile;
static jfieldID s_dataId;

void NvAPKInit()
{
    __android_log_print(ANDROID_LOG_DEBUG, "apk",  "apk init\n");
	JNIEnv *env = NVThreadGetCurrentJNIEnv();
    __android_log_print(ANDROID_LOG_DEBUG, "apk",  "env = %p\n", env);
	fileHelper = (*env)->FindClass(env, "com/nvidia/devtech/NvAPKFileHelper");
    __android_log_print(ANDROID_LOG_DEBUG, "apk",  "class = %d\n", fileHelper);
	jmethodID getInstance = (*env)->GetStaticMethodID(env, fileHelper, "getInstance", "()Lcom/nvidia/devtech/NvAPKFileHelper;");
    __android_log_print(ANDROID_LOG_DEBUG, "apk",  "inst = %d\n", getInstance);
	NvAPKFileClass = (*env)->FindClass(env, "com/nvidia/devtech/NvAPKFileHelper$NvAPKFile");
	
    s_openFile = (*env)->GetMethodID(env, fileHelper, "openFileAndroid", "(Ljava/lang/String;)Lcom/nvidia/devtech/NvAPKFileHelper$NvAPKFile;");
    s_closeFile = (*env)->GetMethodID(env, fileHelper, "closeFileAndroid", "(Lcom/nvidia/devtech/NvAPKFileHelper$NvAPKFile;)V");
    s_lengthId = (*env)->GetFieldID(env, NvAPKFileClass, "length", "I");
    s_seekFile = (*env)->GetMethodID(env, fileHelper, "seekFileAndroid", "(Lcom/nvidia/devtech/NvAPKFileHelper$NvAPKFile;I)J");
    s_positionId = (*env)->GetFieldID(env, NvAPKFileClass, "position", "I");
    s_readFile = (*env)->GetMethodID(env, fileHelper, "readFileAndroid", "(Lcom/nvidia/devtech/NvAPKFileHelper$NvAPKFile;I)V");
    s_dataId = (*env)->GetFieldID(env, NvAPKFileClass, "data", "[B");

	jobject thiz = (*env)->CallStaticObjectMethod(env, fileHelper, getInstance);
	s_globalThiz = (*env)->NewGlobalRef(env, thiz);
}

NvAPKFile* NvAPKOpen(char const* path)
{
	JNIEnv *env = NVThreadGetCurrentJNIEnv();
    jstring test = (*env)->NewStringUTF(env, path);
    jobject fileHandle = (*env)->CallObjectMethod(env, s_globalThiz, s_openFile, test);
    (*env)->DeleteLocalRef(env, (jobject)test);

    return (NvAPKFile *) fileHandle;
}

void NvAPKClose(NvAPKFile* file)
{
	JNIEnv *env = NVThreadGetCurrentJNIEnv();
    (*env)->CallVoidMethod(env, s_globalThiz, s_closeFile, (jobject) file);
    (*env)->DeleteLocalRef(env, (jobject)file);
}

int NvAPKGetc(NvAPKFile *stream)
{
    char buff;
    NvAPKRead(&buff,1,1,stream);
    return buff;
}

char* NvAPKGets(char* s, int size, NvAPKFile* stream)
{
    int i;
    char *d=s;
    for(i = 0; (size > 1) && (!NvAPKEOF(stream)); i++, size--, d++)
    {
        NvAPKRead(d,1,1,stream);
        if(*d==10)
        {
            size=1;
        }
    }
    *d=0;

    return s;
}

size_t NvAPKSize(NvAPKFile* stream)
{
	JNIEnv *env = NVThreadGetCurrentJNIEnv();
    jint len = (*env)->GetIntField(env, (jobject) stream, s_lengthId);

    return len;
}

long NvAPKSeek(NvAPKFile* stream, long offset, int type)
{
	JNIEnv *env = NVThreadGetCurrentJNIEnv();
	switch (type)
	{
	    case SEEK_CUR:
	        offset += NvAPKTell(stream);
	        break;
	    case SEEK_END:
	        offset += NvAPKSize(stream);
            break;
	    case SEEK_SET:
	        // No need to change the offset..
	        break;
	}

    jlong ret = (*env)->CallLongMethod(env, s_globalThiz, s_seekFile, (jobject) stream, (jint) offset);

    return ret;
}

long NvAPKTell(NvAPKFile* stream)
{
	JNIEnv *env = NVThreadGetCurrentJNIEnv();
    jint pos = (*env)->GetIntField(env, (jobject) stream, s_positionId);

    return pos;
}

size_t NvAPKRead(void* ptr, size_t size, size_t nmemb, NvAPKFile* stream)
{
	JNIEnv *env = NVThreadGetCurrentJNIEnv();
    jint readLength = size*nmemb;

    int avail = NvAPKSize(stream)-NvAPKTell(stream);
    if(readLength>avail)
    {
        readLength = avail;
        nmemb = readLength/size;
    }

    (*env)->CallVoidMethod(env, s_globalThiz, s_readFile, (jobject) stream, (jint) readLength);

    jbyteArray data = (jbyteArray) (*env)->GetObjectField(env, (jobject) stream, s_dataId);
    char *data2 = (char *) (*env)->GetByteArrayElements(env, data, NULL);
    memcpy(ptr,data2,readLength);
    (*env)->ReleaseByteArrayElements(env, data, (jbyte*) data2, 0);

    (*env)->DeleteLocalRef(env, data);

    return nmemb;
}

int NvAPKEOF(NvAPKFile *stream)
{
    int rv = (NvAPKTell(stream) >= NvAPKSize(stream)) ? 1 : 0;
    return rv;
}
