LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE	:= xprender

LOCAL_SRC_FILES	+= xprender/Buffer.gl.c
LOCAL_SRC_FILES	+= xprender/Framework.android.c
LOCAL_SRC_FILES	+= xprender/GpuState.gl.c
LOCAL_SRC_FILES	+= xprender/Mat44.c
LOCAL_SRC_FILES	+= xprender/Mat44.gl.c
LOCAL_SRC_FILES	+= xprender/Memory.c
LOCAL_SRC_FILES	+= xprender/NvpParser.c
LOCAL_SRC_FILES	+= xprender/Platform.c
LOCAL_SRC_FILES	+= xprender/RenderTarget.gl.c
LOCAL_SRC_FILES	+= xprender/Shader.gl.c
LOCAL_SRC_FILES	+= xprender/StrHash.c
LOCAL_SRC_FILES	+= xprender/Texture.gl.c
LOCAL_SRC_FILES	+= xprender/Vec2.c
LOCAL_SRC_FILES	+= xprender/Vec3.c
LOCAL_SRC_FILES	+= xprender/Vec4.c

LOCAL_SRC_FILES	+= glsw/glsw.c
LOCAL_SRC_FILES	+= glsw/bstrlib.c

LOCAL_SRC_FILES	+= httpd/api.c
LOCAL_SRC_FILES	+= httpd/ember.c
LOCAL_SRC_FILES	+= httpd/ip_acl.c
LOCAL_SRC_FILES	+= httpd/protocol.c
LOCAL_SRC_FILES	+= httpd/version.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/xprender $(LOCAL_PATH)/pez
LOCAL_CFLAGS := -DXPR_ANDROID

LOCAL_STATIC_LIBRARIES := android_native_app_glue

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)
LOCAL_EXPORT_CFLAGS := $(LOCAL_CFLAGS)
LOCAL_EXPORT_LDLIBS := -llog -landroid -lEGL -lGLESv2

include $(BUILD_STATIC_LIBRARY)
