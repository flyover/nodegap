LOCAL_PATH := $(call my-dir)

MAIN_LOCAL_PATH := $(LOCAL_PATH)

include $(MAIN_LOCAL_PATH)/../../../../node/node-android/Android.mk

LOCAL_PATH := $(MAIN_LOCAL_PATH)

include $(CLEAR_VARS)

LOCAL_MODULE    := NodeGap
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../node_modules/nan
LOCAL_SRC_FILES := NodeGap.cpp
LOCAL_SRC_FILES += NodePlugin.cpp
LOCAL_LDLIBS    := -llog
LOCAL_WHOLE_STATIC_LIBRARIES := node

include $(BUILD_SHARED_LIBRARY)
