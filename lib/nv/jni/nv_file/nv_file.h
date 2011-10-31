//----------------------------------------------------------------------------------
// File:            libs\jni\nv_file\nv_file.h
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

#ifndef __INCLUDED_NV_FILE_H
#define __INCLUDED_NV_FILE_H

#if defined(__cplusplus)
extern "C"
{
#endif

/** @file nv_file.h
  The NvF(ile) library is an abstraction library that makes it possible to
  automatically find files either in Android's /data tree or in the APK
  itself.  The library provides a unified FILE*-like interface to both of
  the file types.  This makes it possible for native applications to migrate
  data between the APK and the /data filesystem without changing application code
  The library uses the nv_apk_file.h system as a part of its implementation
  @see nv_apk_file.h
*/
#include <stdio.h>

/**
 An opaque handle to a file, either in the APK or in the filesystem
 */
typedef void NvFile;

/**
  Initializes the library.  This function MUST be called from the application's
  JNI_OnLoad, from a function known to be called by JNI_OnLoad, or from a function
  in a Java-called thread.  nv_thread-created native threads cannot call this
  initialization function.  Calls NvAPKInit() internally.
  @see nv_apk_file.h
  @see nv_thread.h
  */
void        NvFInit();

/**
  A wrapper similar to fopen.  Only provides read-access to files, since the
  file returned may be in the read-only APK.  Can be called from any 
  JNI-connected thread.
  @param path The path to the file.  This path is searched within /data and 
  within the APK
  @return A pointer to an open file on success, NULL on failure
  */
NvFile*     NvFOpen(char const* path);

/**
  A wrapper similar to fclose.  Can be called from any JNI-connected thread.
  @param file A pointer to an open file opened via NvFOpen()
  */
void        NvFClose(NvFile* file);

/**
  A wrapper similar to chdir.  Can be called from any thread.
  @param dir String path to be made current
  */
void        NvFChdir(const char* dir);


/**
  A wrapper similar to fgetc.  Can be called from any JNI-connected thread.
  @param stream A pointer to an open file opened via NvFOpen()
  @return The character read from the file
  */
int         NvFGetc(NvFile *stream);


/**
  A wrapper similar to fgets.  Can be called from any JNI-connected thread.
  @param s A char buffer to receive the string
  @param size The size of the buffer pointed to by s
  @param stream A pointer to an open file opened via NvFOpen()
  @return A pointer to s on success or NULL on failure
  */
char*       NvFGets(char* s, int size, NvFile* stream);

/**
  Gets the size of the file in bytes.  Can be called from any JNI-connected thread.
  @param stream A pointer to an open file opened via NvFOpen()
  @return The size of the file in bytes
  */
size_t      NvFSize(NvFile* stream);

/**
  A wrapper equivalent to fseek.  Can be called from any JNI-connected thread.
  @param stream A pointer to an open file opened via NvFOpen()
  @param offset The offset from the specified base
  @param type The offset base; same as calls to fseek
  @return Zero on success, nonzero on failure
  */
long        NvFSeek(NvFile* stream, long offset, int type);

/**
  Gets the current file pointer offset.  Can be called from any JNI-connected thread.
  @param stream A pointer to an open file opened via NvFOpen()
  @return The offset of the file pointer in the file in bytes
  */
long        NvFTell(NvFile* stream);

/**
  A wrapper similar to fread.  Can be called from any JNI-connected thread.
  @param ptr A buffer of size size into which data will be read
  @param size size of element to be read
  @param nmemb count of elements to be read
  @param stream A pointer to an open file opened via NvFOpen()
  @return The number of elements read
  */
size_t      NvFRead(void* ptr, size_t size, size_t nmemb, NvFile* stream);

/**
  A wrapper similar to feof.  Can be called from any JNI-connected thread.
  @param stream A pointer to an open file opened via NvFOpen()
  @return Nonzero on EOF, zero otherwise
  */
int         NvFEOF(NvFile *stream);

#if defined(__cplusplus)
}
#endif


#endif
