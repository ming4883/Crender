//----------------------------------------------------------------------------------
// File:            libs\jni\nv_math\nv_math.h
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
#ifndef _NV_MATH_H
#define _NV_MATH_H

#include <GLES2/gl2.h>

GLfloat NvRandf();

/* These are the recommended "safer" ways to use GLfloat for time */

/* Returns the seconds between the two given times */
GLfloat NvClockDiffInSecs(long long int newTime, long long int oldTime);

#define NV_MILISECS_TO_UST(ms) ((long long int)(ms) * 1000000)

#endif

