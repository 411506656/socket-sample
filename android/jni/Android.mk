LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
SOCKET_SERVER_SRC := \
   socket_server.c 

LOCAL_MODULE := socket_server

LOCAL_SRC_FILES := $(addprefix ../../src/, $(SOCKET_SERVER_SRC))

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../../src/ 

#LOCAL_STATIC_LIBRARIES += libplist libimobiledevice libusbmuxd_new libusb1.0 libssl libcrypto libdecrepit
#LOCAL_SHARED_LIBRARIES += libssl libcrypto libdecrepit
LOCAL_CFLAGS += -pie -fPIE -g
LOCAL_LDFLAGS += -pie -fPIE -llog

include $(BUILD_EXECUTABLE)
