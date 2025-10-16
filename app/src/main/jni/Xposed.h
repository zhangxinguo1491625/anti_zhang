//
// Created by 22812 on 2025/10/16.
//

#ifndef ZHANG_ANTI_XPOSED_H
#define ZHANG_ANTI_XPOSED_H
#include <jni.h>
#include <string>

namespace CXposed {
    bool Check_Xposed(JNIEnv* env);
    bool Check_Xposed_class(JNIEnv* env);
    bool Check_Xposed_file(JNIEnv* env);
    jobject Load_Class(JNIEnv* env,const std::string& className);
} // CXposed

#endif //ZHANG_ANTI_XPOSED_H
