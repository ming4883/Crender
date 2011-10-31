LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := crender-mesh-example

LOCAL_SRC_FILES += ../../../Mesh.example.c
LOCAL_SRC_FILES += ../../../Common.c
LOCAL_SRC_FILES += ../../../Framework.android.nv.cpp
LOCAL_SRC_FILES += ../../../Material.c
LOCAL_SRC_FILES += ../../../Mesh.c
LOCAL_SRC_FILES += ../../../Mesh.obj.c
LOCAL_SRC_FILES += ../../../Pvr.c
#LOCAL_SRC_FILES += ../../../Remote.c
#LOCAL_SRC_FILES += ../../../Remote.impl.android.c
LOCAL_SRC_FILES += ../../../Stream.c

LOCAL_STATIC_LIBRARIES := crender_nv nv

include $(BUILD_SHARED_LIBRARY)

$(call import-module, lib)