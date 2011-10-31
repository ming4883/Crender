//----------------------------------------------------------------------------------
// File:            libs\jni\nv_math\misc.h
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

#ifndef INCLUDED_NV_MATH_CPP_MISC_H
#define INCLUDED_NV_MATH_CPP_MISC_H


inline bool nvIsPowerOfTwo(unsigned int i)
{
    return (i & (i - 1)) == 0;
}

inline float nvDegToRadf(float d)
{
    return d * 3.14159265358979323846f / 180.0f;
}

inline float nvRadToDegf(float r)
{
    return r * 180.0f / 3.14159265358979323846f;
}



/*
  'mod' differs from '%' in that it behaves correctly when either the
  numerator or denominator is negative.
*/

inline int nvMod(int n, int d)
{
  int m = n % d; 
  return (((m<0) && (d>0))||
	  ((m>0) && (d<0)))?(m+d):m;
}

inline int nvAbs(int n) {
    if (n<0) return -n;
    return n;
}

inline int nvSign(int n) {
    if (n>0) return 1;
    if (n<0) return -1;
    return 0;
}

/* 
   This returns the smallest amplitude value x such that 
   nvMod(b + x, m) == a 
*/

inline int nvDifMod(int a, int b, int m)
{

    int x1 = a - b;
    int x2 = (x1>0) ? x1 - m : x1 + m;
    return (nvAbs(x1)<nvAbs(x2)) ? x1 : x2;
}


inline float nvWrapf(float a, float min, float max)
{

    assert(max>min);

    float d = max-min;
    float s = a - min;
    float q = s/d;
    float m = q - floorf(q);
    return m * d + min;
}


inline float nvClampf(float a, float min, float max)
{
    return (a<min)?min:((a>max)?max:a);
}


inline int nvClampi(int i, int min, int max)
{
    return (i<min)?min:((i>max)?max:i);
}


inline float nvGaussian(float x, float s) 
{
    float c = s * sqrtf(2.0f * 3.14159265358979323846f);
    return expf(-(x * x)/(2.0f * s * s))/c;
}


inline float nvLerpf(float a, float b, float t)
{
    return a * (1.0f - t) + b * t;
}

inline float nvEasef(float t)
{

    float t_2 = t * t;
    float t_3 = t_2 * t;
    return 3.0f * t_2 - 2.0f * t_3;
}


#endif

