#include <jni.h>
#include <android/log.h>
#include "root_find.h"
#include "EmulatorChecker.h"
#include "MagiskChecker.h"
#include "FridaChecker.h"
#include "DualChecker.h"
#include "DeviceInfo.h"
#include "Xposed.h"
#include "Debugger_checker.h"
#include "Unicorn_Check.h"

#define LOG_TAG "MyNativeTag"

// Warning 级别
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)

// 其他级别也可以自定义
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

jobject globalContext = nullptr;
JNIEnv* g_env = nullptr;
JavaVM* g_jvm = nullptr; //用于后面获取JNIEnv
static jboolean get_Context(JNIEnv* env, jclass, jobject context) {
    globalContext = env->NewGlobalRef(context);
    return JNI_TRUE;
}

static jstring getDeviceInfo(JNIEnv* env,  jclass) {
    return CDeviceInfo::Check_DeviceInfo(env);
}

static jboolean checkEmu(JNIEnv* env,  jclass) {
    return CEmulatorChecker::Check_Emulator() ? JNI_TRUE : JNI_FALSE;
}

static jboolean checkUnicorn(JNIEnv* env,  jclass) {
    return Unicorn_Check::Check_Unicorn(env) ? JNI_TRUE : JNI_FALSE;
}

static jboolean checkDebugger(JNIEnv* env,  jclass) {
    return Debugger_checker::Check_Debugger() ? JNI_TRUE : JNI_FALSE;
}

static jboolean checkRoot(JNIEnv* env, jclass) {
    CRootChecker checker;
    return checker.Check_Root() ? JNI_TRUE : JNI_FALSE;
}

static jboolean checkBox(JNIEnv* env, jclass) {
    return CDualChecker::Check_Dual() ? JNI_TRUE : JNI_FALSE;
}

static jboolean checkMagisk(JNIEnv* env, jclass) {
    return CMagiskChecker::Check_Magisk() ? JNI_TRUE : JNI_FALSE;
}

static jboolean checkFrida(JNIEnv* env, jclass) {
    return CFridaChecker::Check_Frida() ? JNI_TRUE : JNI_FALSE;
}

static jboolean checkXposed(JNIEnv* env, jclass) {
    return CXposed::Check_Xposed(env) ? JNI_TRUE : JNI_FALSE;
}

static JNINativeMethod nativeMethods[] = {
        {"n0",  "(Landroid/content/Context;)Z", reinterpret_cast<void *>(get_Context)},
        {"n3",  "()Z", reinterpret_cast<void *>(checkUnicorn)},
        {"n5",  "()Z", reinterpret_cast<void *>(checkDebugger)},
        {"n6",  "()Ljava/lang/String;", reinterpret_cast<void *>(getDeviceInfo)},
        {"n8",  "()Z", reinterpret_cast<void *>(checkRoot)},
        {"n9",  "()Z", reinterpret_cast<void *>(checkBox)},
        {"n10", "()Z", reinterpret_cast<void *>(checkEmu)},
        {"n12", "()Z", reinterpret_cast<void *>(checkMagisk)},
        {"n13", "()Z", reinterpret_cast<void *>(checkFrida)},
        {"n14", "()Z", reinterpret_cast<void *>(checkXposed)}
};

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    __android_log_print(ANDROID_LOG_DEBUG, "JNI_ANTI", "JNI_OnLoad called");
    if (JNI_OK != vm->GetEnv(reinterpret_cast<void**> (&g_env),JNI_VERSION_1_6)) {
        __android_log_print(ANDROID_LOG_DEBUG, "JNI_ANTI", "JNI_OnLoad could not get JNI env");
        return JNI_ERR;
    }
    g_jvm = vm; //用于后面获取JNIEnv
    jclass clazz = g_env->FindClass("com/example/zhang_anti/AntiNative");  //获取Java NativeLib类
    if (clazz == nullptr){
        LOGW("clazz not find");
        return JNI_FALSE;
    }
    //注册Native方法
    if (g_env->RegisterNatives(clazz, nativeMethods, sizeof(nativeMethods)/sizeof((nativeMethods)[0])) < 0) {
        LOGW("RegisterNatives error");
        return JNI_ERR;
    }
    __android_log_print(ANDROID_LOG_DEBUG, "JNI_ANTI", "[BridgeHelp]registerNativeMethod");
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL JNI_OnUnload(__unused JavaVM *, __unused void *) {
    ;
}
