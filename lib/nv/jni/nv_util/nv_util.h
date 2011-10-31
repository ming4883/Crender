//----------------------------------------------------------------------------------
// File:            libs\jni\nv_util\nv_util.h
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
#ifndef __INCLUDED_NV_UTIL_H
#define __INCLUDED_NV_UTIL_H

/** @file nv_util.h
 * Used to set and retrieve Java parameter and shader values from
 * within native code.  Both command-line arguments and predefined
 * shared values, such as the location of the external storage are
 * available
 */

/**
  Initializes the library.  This function MUST be called from the application's
  JNI_OnLoad, from a function known to be called by JNI_OnLoad, or from a function
  in a Java-called thread.  nv_thread-created native threads cannot call this
  initialization function.
  @see nv_thread.h
  */
void NvUtilInit();

/**
 * Used to retrieve Java-set string values in native code
 * Pre-set values include:
 * <ul>
 *  <li> STORAGE_ROOT: The full path to the external storage device (SD card)
 * </ul>
 * Gets the specified key value from the app local value list
 * @param buffer The buffer to receive the value's string value
 * @param bufferLength the size of buffer in bytes
 * @param The name of the value to retrieve
 */
void NvUtilGetLocalAppValue(char *buffer, int bufferLength, const char* name);

/**
 * Used to verify Java-set string values in native code
 * @param name The name of the value to verify
 * @return true if the value exists, false if not
 */
bool NvUtilHasLocalAppValue(const char* name);

/**
 * Used to share string values between native and Java code
 * Sets the specified key value in the app local value list
 * @param name The key to set the value of
 * @param value The value
 */
void NvUtilSetLocalAppValue(const char* name, const char* value);

/**
 * A JNI-accessible version of NvUtil::getParameter.  This function is used to get the parameters used to start the Activity via, for example:
 * <pre>
 * adb shell am start -a android.intent.action.MAIN -n com.nvidia.devtech.water/com.nvidia.devtech.water.Water -e param1 1 -e param2 2
 * </pre>
 * Where "param1" and "param2" are the parameter names and "1" and "2" are the parameter values.
 *  
 * @param buffer The buffer to receive the parameter's string value
 * @param bufferLength the size of buffer in bytes
 * @param The name of the parameter to retrieve
 */
void NvUtilGetParameter(char *buffer, int bufferLength, const char *parameter);


#endif
