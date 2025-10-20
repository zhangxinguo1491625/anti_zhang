//
// Created by 22812 on 2025/10/20.
//

#include <jni.h>
#include "Unicorn_Check.h"
#include "android/log.h"
#include <sys/utsname.h>
#include <string>

namespace Unicorn_Check {
    bool Check_Unicorn(JNIEnv* env){
        if(checkUnicornByUname()||checkUnicornByStaticMethodId(env)||checkUnicornByStaticMethodHash(env)) {
            return true;
        }
    }
    bool checkUnicornByUname(){
        struct utsname u;
        uname(&u);
        if (strstr(u.release, "unidbg")) {
            __android_log_print(ANDROID_LOG_DEBUG,"checkUnicornByUname","Unidbg detected by uname release=%s", u.release);
            return true;
        }
        return false;
    }
    int java_hashCode(const char *str) {
        int hash = 0;
        for (int i = 0; i < strlen(str); i++) {
            hash = 31 * hash + str[i];
        }
        return hash;
    }
    bool checkUnicornByStaticMethodId(JNIEnv* env){
        jclass process_class = env->FindClass("android/os/Process");
        jmethodID wrong_methodid = env->GetStaticMethodID(process_class,"ntestmdXPWS","()V");
        if (env->ExceptionCheck()){
            env->ExceptionClear();  // 清除异常
        }
        if (wrong_methodid != 0) {
            __android_log_print(ANDROID_LOG_DEBUG,"checkUnicornByStaticMethodId","Unidbg detected by GetStaticMethodID(%s)", "ntestmdXPWS");
            return true;
        }
        return false;
    }
    bool checkUnicornByStaticMethodHash(JNIEnv* env){
        jclass process_class = env->FindClass("android/os/Process");
        jmethodID wrong_methodid = env->GetStaticMethodID(process_class,"myPid","()I");
        jmethodID wrong_methodid1 = env->GetStaticMethodID(process_class,"myPid","()I");
        if((long)wrong_methodid == java_hashCode("android/os/Process->myPid()I")||(wrong_methodid == wrong_methodid1 && (uintptr_t)wrong_methodid < 0x10000000ULL)){
            return true;
        }
        return false;
    }
} // Unicorn_Check