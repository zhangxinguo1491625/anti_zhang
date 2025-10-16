LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := anti

LOCAL_SRC_FILES := \
    main.cpp \
    Common.cpp \
    root_find.cpp \
    EmulatorChecker.cpp \
    DualChecker.cpp \
    MagiskChecker.cpp \
    FridaChecker.cpp \
    elfutil.cpp \
    MemCommon.cpp \
    DeviceInfo.cpp \
    Common_Sensor.cpp \
    Xposed.cpp \
    ElfFileReader.cpp

LOCAL_CPPFLAGS := -std=c++17
LOCAL_LDLIBS := -llog -landroid     # 不要 -lc++

include $(BUILD_SHARED_LIBRARY)