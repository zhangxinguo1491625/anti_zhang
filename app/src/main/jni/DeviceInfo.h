//
// Created by 22812 on 2025/10/9.
//

#include <jni.h>
#include <string.h>
#ifndef ZHANG_ANTI_DEVICEINFO_H
#define ZHANG_ANTI_DEVICEINFO_H
#include <map>
#include <string>

extern jobject globalContext;
extern JNIEnv* g_env;
namespace CDeviceInfo {
    jstring Check_DeviceInfo(JNIEnv* env);
    std::string get_Secure_id(JNIEnv* env, std::string & key);
    std::string get_Global_id(JNIEnv* env, std::string & key);
    std::string get_fingerprint(JNIEnv *env);
    std::string get_serial(JNIEnv *env);
    std::string get_ssh();
    std::string get_bootload();
    std::string get_vpn(JNIEnv* env);
    std::string get_proxy();
    std::string get_network_list(JNIEnv* env);
    std::string enabled_accessibility_package(JNIEnv* env);
    std::string get_sim_property();
    std::string get_package(JNIEnv* env);
    std::string getPackagePath(JNIEnv* env);
    std::string getPackagedatafile(JNIEnv* env);
    bool getPackagedatafile_virtual();
    jint sdk_int(JNIEnv* env);
} // CDeviceInfo

#endif //ZHANG_ANTI_DEVICEINFO_H
