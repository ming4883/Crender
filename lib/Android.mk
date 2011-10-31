LOCAL_PATH:= $(call my-dir)

#
# nvevent
#
include $(CLEAR_VARS)

LOCAL_MODULE := nv

LOCAL_SRC_FILES	+= nv/jni/nv_event/nv_event.cpp
LOCAL_SRC_FILES	+= nv/jni/nv_thread/nv_thread.c
LOCAL_SRC_FILES	+= nv/jni/nv_time/nv_time.cpp
LOCAL_SRC_FILES	+= nv/jni/nv_apk_file/nv_apk_file.c
LOCAL_SRC_FILES	+= nv/jni/nv_file/nv_file.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/nv/jni
LOCAL_CFLAGS := -D_TIMFUDGE -DSUPPORT_APK

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_EXPORT_CFLAGS := $(LOCAL_CFLAGS)

include $(BUILD_STATIC_LIBRARY)

#
# crender_xx
#
CRENDER_FILES	+= crender/Buffer.gl.c
CRENDER_FILES	+= crender/Context.gl.c
CRENDER_FILES	+= crender/Mat44.c
CRENDER_FILES	+= crender/Mat44.gl.c
CRENDER_FILES	+= crender/Mem.c
CRENDER_FILES	+= crender/NvpParser.c
CRENDER_FILES	+= crender/Platform.c
CRENDER_FILES	+= crender/Shader.gl.c
CRENDER_FILES	+= crender/StrHash.c
CRENDER_FILES	+= crender/Texture.gl.c
CRENDER_FILES	+= crender/Vec2.c
CRENDER_FILES	+= crender/Vec3.c
CRENDER_FILES	+= crender/Vec4.c

CRENDER_FILES	+= glsw/glsw.c
CRENDER_FILES	+= glsw/bstrlib.c

#CRENDER_FILES	+= httpd/api.c
#CRENDER_FILES	+= httpd/ember.c
#CRENDER_FILES	+= httpd/ip_acl.c
#CRENDER_FILES	+= httpd/protocol.c
#CRENDER_FILES	+= httpd/version.c

# crender_nv
include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/crender
LOCAL_CFLAGS := -DCR_ANDROID

LOCAL_MODULE	:= crender_nv

LOCAL_SRC_FILES	:= $(CRENDER_FILES)
LOCAL_SRC_FILES	+= crender/Context.android.nv.cpp

LOCAL_STATIC_LIBRARIES := nv
LOCAL_LDLIBS := -llog -lEGL -lGLESv2

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_EXPORT_CFLAGS := $(LOCAL_CFLAGS)
LOCAL_EXPORT_LDLIBS := $(LOCAL_LDLIBS)

include $(BUILD_STATIC_LIBRARY)

# crender
include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/crender
LOCAL_CFLAGS := -DCR_ANDROID

LOCAL_MODULE	:= crender

LOCAL_SRC_FILES	:= $(CRENDER_FILES)
LOCAL_SRC_FILES	+= crender/Context.android.c

LOCAL_STATIC_LIBRARIES := android_native_app_glue
LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv2

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_EXPORT_CFLAGS := $(LOCAL_CFLAGS)
LOCAL_EXPORT_LDLIBS := $(LOCAL_LDLIBS)

include $(BUILD_STATIC_LIBRARY)


