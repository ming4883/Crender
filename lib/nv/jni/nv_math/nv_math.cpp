//----------------------------------------------------------------------------------
// File:            libs\jni\nv_math\nv_math.cpp
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

#include "nv_math.h"

static const double a = 16807.0;
static const double m = 2147483647.0;

static double nextSeed(double seed) {
	double t = a * seed;
	return t - m * (double)((int)(t/m));
}

GLfloat NvRandf()
{
    static double seed = 1.0;
    seed = nextSeed(seed);
    return (GLfloat) (seed * (1/m));
}

GLfloat NvClockDiffInSecs(long long int newTime, long long int oldTime)
{
    // kdGetTimeUST is never supposed to decrease (i.e. that
    // means it can't wrap, either.  IIRC, it will wrap in
    // 593 years).  So we ignore that option...

    // Used for frame-to-frame diffs, this will fit in 32b
    // as long as the times are < ~4.3s apart
	long long int diffTime = newTime - oldTime;

    // Need to find a better way to do this.
    // However, I believe we will be uSec precise (not run
    // out of mantissa bits for uSecs) as long as the diff
    // is < ~16s  (1e-6 * (2^24))
    return ((GLfloat)diffTime) / 1.0e9f;
}
