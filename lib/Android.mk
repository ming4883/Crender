LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE	:= crender

LOCAL_SRC_FILES	+= crender/Buffer.gl.c
LOCAL_SRC_FILES	+= crender/Context.android.c
LOCAL_SRC_FILES	+= crender/Context.gl.c
//LOCAL_SRC_FILES	+= crender/GpuState.gl.c
LOCAL_SRC_FILES	+= crender/Mat44.c
LOCAL_SRC_FILES	+= crender/Mat44.gl.c
LOCAL_SRC_FILES	+= crender/Mem.c
LOCAL_SRC_FILES	+= crender/NvpParser.c
LOCAL_SRC_FILES	+= crender/Platform.c
LOCAL_SRC_FILES	+= crender/RenderTarget.gl.c
LOCAL_SRC_FILES	+= crender/Shader.gl.c
LOCAL_SRC_FILES	+= crender/StrHash.c
LOCAL_SRC_FILES	+= crender/Texture.gl.c
LOCAL_SRC_FILES	+= crender/Vec2.c
LOCAL_SRC_FILES	+= crender/Vec3.c
LOCAL_SRC_FILES	+= crender/Vec4.c

LOCAL_SRC_FILES	+= glsw/glsw.c
LOCAL_SRC_FILES	+= glsw/bstrlib.c

LOCAL_SRC_FILES	+= httpd/api.c
LOCAL_SRC_FILES	+= httpd/ember.c
LOCAL_SRC_FILES	+= httpd/ip_acl.c
LOCAL_SRC_FILES	+= httpd/protocol.c
LOCAL_SRC_FILES	+= httpd/version.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/crender $(LOCAL_PATH)/pez
LOCAL_CFLAGS := -DCR_ANDROID

LOCAL_STATIC_LIBRARIES := android_native_app_glue
LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv2

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_EXPORT_CFLAGS := $(LOCAL_CFLAGS)
LOCAL_EXPORT_LDLIBS := $(LOCAL_LDLIBS)

include $(BUILD_STATIC_LIBRARY)
#include $(BUILD_SHARED_LIBRARY)
