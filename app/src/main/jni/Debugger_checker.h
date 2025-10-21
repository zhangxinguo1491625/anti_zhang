//
// Created by 22812 on 2025/10/20.
//

#ifndef ZHANG_ANTI_DEBUGGER_CHECKER_H
#define ZHANG_ANTI_DEBUGGER_CHECKER_H
#include <jni.h>

extern JNIEnv* g_env;
namespace Debugger_checker {
    bool Check_Debugger();
    bool isDebuggerDetectedByPort();
    bool isDebuggerDetectedByAttach();
    bool isJavaDebuggerConnected();
    bool isDebuggerDetectedByLinker();
} // Debugger_checker

#endif //ZHANG_ANTI_DEBUGGER_CHECKER_H
