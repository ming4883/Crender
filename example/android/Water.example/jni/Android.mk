LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := crender-water-example

LOCAL_SRC_FILES += ../../../Water.example.c
#LOCAL_SRC_FILES += ../../../Reflection.example.c
LOCAL_SRC_FILES += ../../../Common.c
LOCAL_SRC_FILES += ../../../Framework.android.c
LOCAL_SRC_FILES += ../../../Material.c
LOCAL_SRC_FILES += ../../../Mesh.c
LOCAL_SRC_FILES += ../../../Mesh.obj.c
LOCAL_SRC_FILES += ../../../Pvr.c
LOCAL_SRC_FILES += ../../../Remote.c
LOCAL_SRC_FILES += ../../../Remote.impl.android.c
LOCAL_SRC_FILES += ../../../Stream.c

LOCAL_STATIC_LIBRARIES := crender android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module, android/native_app_glue)
$(call import-module, lib)