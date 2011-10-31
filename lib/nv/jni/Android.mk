#----------------------------------------------------------------------------------
# File:            libs\jni\Android.mk
# Samples Version: Android NVIDIA samples 2 
# Email:           tegradev@nvidia.com
# Forum:           http://developer.nvidia.com/tegra/forums/tegra-forums/android-development
#
# Copyright 2009-2010 NVIDIA® Corporation 
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#----------------------------------------------------------------------------------

include $(CLEAR_VARS)

LOCAL_PATH := nv_math
LOCAL_MODULE := nvmath
LOCAL_SRC_FILES := nv_math.cpp nv_matrix.cpp nv_quat.cpp
LOCAL_ARM_MODE   := arm
LOCAL_LDFLAGS +=  -L../../../libs/obj/local/$(TARGET_ARCH_ABI) -L../../build/platforms/tegra/lib
LOCAL_C_INCLUDES := ../../../libs/jni/ ../../build/platforms/tegra/include

include $(BUILD_STATIC_LIBRARY)


include $(CLEAR_VARS)

LOCAL_PATH := nv_shader
LOCAL_MODULE := nvshader
LOCAL_SRC_FILES := nv_shader.cpp
LOCAL_ARM_MODE   := arm
LOCAL_LDFLAGS +=  -L../../../libs/obj/local/$(TARGET_ARCH_ABI) -L../../build/platforms/tegra/lib
LOCAL_C_INCLUDES := ../../../libs/jni/ ../../build/platforms/tegra/include

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_PATH := nv_apk_file
LOCAL_MODULE := nvapkfile
LOCAL_SRC_FILES := nv_apk_file.c
LOCAL_ARM_MODE   := arm
LOCAL_LDFLAGS +=  -L../../../libs/obj/local/$(TARGET_ARCH_ABI) -L../../build/platforms/tegra/lib
LOCAL_C_INCLUDES := ../../../libs/jni/ ../../build/platforms/tegra/include

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_PATH := nv_util
LOCAL_MODULE := nvutil
LOCAL_SRC_FILES := nv_util.cpp
LOCAL_ARM_MODE   := arm
LOCAL_LDFLAGS +=  -L../../../libs/obj/local/$(TARGET_ARCH_ABI) -L../../build/platforms/tegra/lib
LOCAL_C_INCLUDES := ../../../libs/jni/ ../../build/platforms/tegra/include

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_PATH := nv_time
LOCAL_MODULE := nvtime
LOCAL_SRC_FILES := nv_time.cpp
LOCAL_ARM_MODE   := arm
LOCAL_LDFLAGS +=  -L../../../libs/obj/local/$(TARGET_ARCH_ABI) -L../../build/platforms/tegra/lib
LOCAL_C_INCLUDES := ../../../libs/jni/ ../../build/platforms/tegra/include

include $(BUILD_STATIC_LIBRARY)


include $(CLEAR_VARS)

LOCAL_PATH := nv_event
LOCAL_MODULE := nvevent
LOCAL_SRC_FILES := nv_event.cpp
LOCAL_ARM_MODE   := arm
LOCAL_LDFLAGS +=  -L../../../libs/obj/local/$(TARGET_ARCH_ABI) -L../../build/platforms/tegra/lib
LOCAL_C_INCLUDES := ../../../libs/jni/ ../../build/platforms/tegra/include

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_PATH := nv_file
LOCAL_MODULE := nvfile
LOCAL_SRC_FILES := nv_file.c
LOCAL_ARM_MODE   := arm
LOCAL_LDFLAGS +=  -L../../../libs/obj/local/$(TARGET_ARCH_ABI) -L../../build/platforms/tegra/lib
LOCAL_C_INCLUDES := ../../../libs/jni/ ../../build/platforms/tegra/include

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_PATH := nv_thread
LOCAL_MODULE := nvthread
LOCAL_SRC_FILES := nv_thread.c
LOCAL_ARM_MODE   := arm
LOCAL_LDFLAGS +=  -L../../../libs/obj/local/$(TARGET_ARCH_ABI) -L../../build/platforms/tegra/lib
LOCAL_C_INCLUDES := ../../../libs/jni/ ../../build/platforms/tegra/include

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_PATH := nv_hhdds
LOCAL_MODULE := nvhhdds
LOCAL_SRC_FILES := nv_hhdds.cpp
LOCAL_ARM_MODE   := arm
LOCAL_LDFLAGS +=  -L../../../libs/obj/local/$(TARGET_ARCH_ABI) -L../../build/platforms/tegra/lib
LOCAL_C_INCLUDES := ../../../libs/jni/ ../../build/platforms/tegra/include

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_PATH := nv_log
LOCAL_MODULE := nvlog
LOCAL_SRC_FILES := nv_log.cpp
LOCAL_ARM_MODE   := arm
LOCAL_LDFLAGS +=  -L../../../libs/obj/local/$(TARGET_ARCH_ABI) -L../../build/platforms/tegra/lib
LOCAL_C_INCLUDES := ../../../libs/jni/ ../../build/platforms/tegra/include

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_PATH := nv_glesutil
LOCAL_MODULE := nvglesutil
LOCAL_SRC_FILES := nv_images.cpp
LOCAL_ARM_MODE   := arm
LOCAL_LDFLAGS +=  -L../../../libs/obj/local/$(TARGET_ARCH_ABI) -L../../build/platforms/tegra/lib
LOCAL_C_INCLUDES := ../../../libs/jni/ ../../build/platforms/tegra/include

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_PATH := nv_sound
LOCAL_MODULE := nvsound
LOCAL_SRC_FILES := nv_sound.cpp
LOCAL_ARM_MODE   := arm
LOCAL_LDFLAGS +=  -L../../../libs/obj/local/$(TARGET_ARCH_ABI) -L../../build/platforms/tegra/lib
LOCAL_C_INCLUDES := ../../../libs/jni/ ../../build/platforms/tegra/include

include $(BUILD_STATIC_LIBRARY)

