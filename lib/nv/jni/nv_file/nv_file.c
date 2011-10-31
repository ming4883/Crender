//----------------------------------------------------------------------------------
// File:            libs\jni\nv_file\nv_file.c
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

#include "nv_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef ANDROID
#include "../nv_apk_file/nv_apk_file.h"
#define SUPPORT_APK 1
#include <unistd.h>
#endif

typedef struct
{
    int     type;
    void   *handle;
} NvFileHandle;

enum
{
    APK_FILE,
    STD_FILE
};

void  NvFInit()
{
#ifdef SUPPORT_APK
    NvAPKInit();
#endif
}

NvFile* NvFOpen(char const* path)
{
    NvFileHandle *handle = NULL;

    // First, try the given path with no mods...
    FILE *fp = fopen(path, "rb");

    // No luck...  Try prepending /data...
    if (!fp)
    {
        char buf[512];
        sprintf(buf, "/data/%s", path); //TODO: use storage card path in the future?
        fp = fopen(buf, "rb");
    }

    if (fp)
    {
        handle = (NvFileHandle*) malloc(sizeof(NvFileHandle));
        handle->type = STD_FILE;
        handle->handle = fp;
    }
#ifdef SUPPORT_APK
    else
    {
        NvAPKFile* apk = NvAPKOpen(path);
        if (apk)
        {
            handle = (NvFileHandle*) malloc(sizeof(NvFileHandle));
            handle->type = APK_FILE;
            handle->handle = NvAPKOpen(path);
        }
    }
#endif
    return (NvFile*) handle;
}

void NvFClose(NvFile* file)
{
    NvFileHandle *handle = (NvFileHandle*) file;
    if (!handle)
        return;

#ifdef SUPPORT_APK
    if (handle->type != STD_FILE)
        NvAPKClose(handle->handle);
    else
#endif
        fclose(handle->handle);
    free(handle);
}

void NvFChdir(const char* dir)
{
    (void)chdir(dir);
}

int NvFGetc(NvFile *stream)
{
    NvFileHandle *handle = (NvFileHandle*) stream;

    if (!handle)
        return -1;

#ifdef SUPPORT_APK
    if (handle->type != STD_FILE)
        return NvAPKGetc(handle->handle);
    else
#endif
        return fgetc(handle->handle);
}

char* NvFGets(char* s, int size, NvFile* stream)
{
    NvFileHandle *handle = (NvFileHandle*) stream;

    if (!handle)
        return NULL;

#ifdef SUPPORT_APK
    if (handle->type != STD_FILE)
        return NvAPKGets(s, size, handle->handle);
    else
#endif
        return fgets(s, size, handle->handle);
}
size_t NvFSize(NvFile* stream)
{
    NvFileHandle *handle = (NvFileHandle*) stream;

    if (!handle)
        return 0;

#ifdef SUPPORT_APK
    if (handle->type != STD_FILE)
    {
        return NvAPKSize(handle->handle);
    }
    else
#endif
    {
        int pos = ftell(handle->handle);
        int size = 0;
        fseek(handle->handle, 0, SEEK_END);
        size = ftell(handle->handle);
        fseek(handle->handle, pos, SEEK_SET);
        return size;
    }
}

long NvFSeek(NvFile* stream, long offset, int type)
{
    NvFileHandle *handle = (NvFileHandle*) stream;

    if (!handle)
        return 0;

#ifdef SUPPORT_APK
    if (handle->type != STD_FILE)
        return NvAPKSeek(handle->handle, offset, type);
    else
#endif
        return fseek(handle->handle, offset, type);
}

long NvFTell(NvFile* stream)
{
    NvFileHandle *handle = (NvFileHandle*) stream;

    if (!handle)
        return 0;

#ifdef SUPPORT_APK
    if (handle->type != STD_FILE)
        return NvAPKTell(handle->handle);
    else
#endif
        return ftell(handle->handle);
}

size_t NvFRead(void* ptr, size_t size, size_t nmemb, NvFile* stream)
{
    NvFileHandle *handle = (NvFileHandle*) stream;

    if (!handle)
        return 0;

#ifdef SUPPORT_APK
    if (handle->type != STD_FILE)
        return NvAPKRead(ptr, size, nmemb, handle->handle);
    else
#endif
        return fread(ptr, size, nmemb, handle->handle);
}

int NvFEOF(NvFile *stream)
{
    NvFileHandle *handle = (NvFileHandle*) stream;

    if (!handle)
        return 0;

#ifdef SUPPORT_APK
    if (handle->type != STD_FILE)
        return NvAPKEOF(handle->handle);
    else
#endif
        return feof(((FILE*)handle->handle));
}
