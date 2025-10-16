//
// Created by 22812 on 2025/10/16.
//

#include "Xposed.h"
#include <jni.h>
#include <string>
namespace CXposed {
    bool Check_Xposed(JNIEnv* env){

        return true;
    }
    jobject Load_Class(JNIEnv* env,std::string className){
        jclass classloader_cls = env->FindClass("java/lang/ClassLoader");
        jmethodID getSystemClassLoader = env->GetStaticMethodID(classloader_cls,"getSystemClassLoader","()Ljava/lang/ClassLoader;");
        jobject Systemclassloader_obj = env->CallStaticObjectMethod(classloader_cls,getSystemClassLoader);
        
        return nullptr;
    }
} // CXposed