//----------------------------------------------------------------------------------
// File:            libs\jni\nv_math\nv_quat.h
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
#ifndef INCLUDED_QUAT_H
#define INCLUDED_QUAT_H

#include <GLES2/gl2.h>
#include <math.h>
#include <assert.h>

void NvQuatCopy(GLfloat r[4], const GLfloat q[4]);
void NvQuatConvertTo3x3Mat(GLfloat r[3][3], const GLfloat q[4]);

void NvQuatIdentity(GLfloat r[4]);

void NvQuatFromAngleAxis(GLfloat r[4], GLfloat radians, const GLfloat axis[3]);

void NvQuatX(GLfloat r[4], GLfloat radians);

void NvQuatY(GLfloat r[4], GLfloat radians);

void NvQuatZ(GLfloat r[4], GLfloat radians);

void NvQuatFromEuler(GLfloat r[4], GLfloat heading,
                     GLfloat pitch, GLfloat roll);

void NvQuatFromEulerReverse(GLfloat r[4], GLfloat heading,
                     GLfloat pitch, GLfloat roll);

GLfloat NvQuatDot(const GLfloat q1[4], const GLfloat q2[4]);

void NvQuatMult(GLfloat r[4], const GLfloat q1[4], const GLfloat q2[4]);

void NvQuatNLerp(GLfloat r[4], const GLfloat q1[4], const GLfloat q2[4], GLfloat t);

void NvQuatNormalize(GLfloat r[4], const GLfloat q[4]);

#endif
