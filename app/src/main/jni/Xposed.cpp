//
// Created by 22812 on 2025/10/16.
//

#include "Xposed.h"
#include <jni.h>
#include <string>
#include <vector>
#include <unistd.h>

namespace CXposed {
    bool Check_Xposed(JNIEnv* env){
        if(Check_Xposed_class(env)) {
            return true;
        }
        return false;
    }
    bool Check_Xposed_class(JNIEnv* env){
        if((Load_Class(env,"de.robv.android.xposed.XposedHelpers")!= nullptr)||
        (Load_Class(env,"de.robv.android.xposed.XposedBridge")!= nullptr)||
        (Load_Class(env,"com.virjar.ratel.api.rposed.RposedHelpers")!= nullptr))
        {
            return true;
        }
        return false;
    }
    bool Check_Xposed_file(JNIEnv* env){
        std::vector<std::string> pathenvs = {
                "/sbin/.magisk/modules/riru_lsposed",
                "/data/adb/lspd",
                "/data/adb/riru/modules/lspd",
                "/sbin/.magisk/modules/riru_edxposed",
                "/data/misc/riru/modules/edxp",
                "/data/adb/riru/modules/edxp.prop",
                "/sbin/.magisk/modules/taichi",
                "/data/misc/taichi",
                "/data/misc/riru/modules/dreamland",
                "/data/adb/riru/modules/dreamland",
                "/system/bin/app_process.orig",
                "/system/xposed.prop",
                "/system/framework/XposedBridge.jar",
                "/system/lib/libxposed_art.so",
                "/system/lib/libxposed_art.so.no_orig",
                "/system/lib64/libxposed_art.so",
                "/system/lib64/libxposed_art.so.no_orig"
        };
        for(auto pathenv:pathenvs){
            if(access(pathenv.c_str(),0)==0){
                return true;
            }
        }
        return false;
    }
    jobject Load_Class(JNIEnv* env,const std::string& className){
        jclass classloader_cls = env->FindClass("java/lang/ClassLoader");
        jmethodID getSystemClassLoader = env->GetStaticMethodID(classloader_cls,"getSystemClassLoader","()Ljava/lang/ClassLoader;");
        jobject Systemclassloader_obj = env->CallStaticObjectMethod(classloader_cls,getSystemClassLoader);

        jstring j_str = env->NewStringUTF(className.c_str());
        jclass Systemclassloader_cls = env->GetObjectClass(Systemclassloader_obj);
        jmethodID loadClass_methodid = env->GetMethodID(Systemclassloader_cls,"loadClass","(Ljava/lang/String;)Ljava/lang/Class;");
        jobject j_clz = env->CallObjectMethod(Systemclassloader_obj,loadClass_methodid,j_str);
        if (env->ExceptionCheck()) {
            env->ExceptionDescribe();
            env->ExceptionClear();
            return nullptr;
        }
        return j_clz;
    }
} // CXposed