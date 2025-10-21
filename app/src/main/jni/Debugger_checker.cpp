//
// Created by 22812 on 2025/10/20.
//

#include "Debugger_checker.h"
#include "elfutil.h"
#include "Common.h"
#include "MemCommon.h"
#include <jni.h>
#include <android/log.h>
#include <sys/uio.h>
#include <string>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>

namespace Debugger_checker {
    bool Check_Debugger(){
        if(isDebuggerDetectedByPort()||isDebuggerDetectedByAttach()||isJavaDebuggerConnected()||isDebuggerDetectedByLinker()) {
            return true;
        }
        return false;
    }
    bool isDebuggerDetectedByPort(){
        FILE* fp = fopen("/proc/net/tcp", "rb");
        if (!fp) return false;
        char line[1024] = { 0 };
        while (fgets(line, sizeof(line) - 1, fp)) {
            if(strstr(line,":5D8A")!=nullptr){
                return true;
            }
        }
        return false;
    }
    bool isDebuggerDetectedByAttach(){
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return false;
        }

        if (pid == 0) {
            pid_t ppid = getppid();
            int res = 0;

            if (ptrace(PTRACE_ATTACH, ppid, NULL, NULL) == 0) {
                __android_log_print(ANDROID_LOG_DEBUG, "isDebuggerDetectedByAttach","[%s] attach %d success", __FUNCTION__, ppid);
                waitpid(ppid, NULL, 0);
                ptrace(PTRACE_CONT, ppid, NULL, NULL);
                ptrace(PTRACE_DETACH, ppid, NULL, NULL);
                res = 0;
            } else {
                __android_log_print(ANDROID_LOG_DEBUG, "isDebuggerDetectedByAttach","[%s] attach fail errno=%d", __FUNCTION__, errno);
                if (errno == EPERM || errno == EBUSY)
                    res = 1; // 被调试器附加
            }

            _exit(res);
        }

        int status = 0;
        waitpid(pid, &status, 0);
        int res = WEXITSTATUS(status);
        return (res == 1);
    }
    bool isJavaDebuggerConnected(){
        //android_os_Debug  _isDebuggerConnected
        jclass Debugcls = g_env->FindClass("android/os/Debug");
        if(Debugcls){
            jmethodID isDebuggerConnected_methodid = g_env->GetStaticMethodID(Debugcls,"isDebuggerConnected","()Z");
            jboolean isDebuggerConnected_bool = g_env->CallStaticBooleanMethod(Debugcls,isDebuggerConnected_methodid);
            return isDebuggerConnected_bool;
        }
        return false;
    }
    bool isDebuggerDetectedByLinker() {
        bool bFound = false;
        elfutil::elfutil* pElfUtil = Common::getElfUtil("/linker");
        if (pElfUtil != nullptr) {
            std::string strTemp("rtld_db_dlactivity");
            unsigned int nSize = 0;
            uint8_t* pSymbol = (uint8_t*)pElfUtil->FindDynamicSymbolAddress(strTemp, &nSize);
            if (pSymbol != nullptr) {
#if defined(__arm__)
                if (((unsigned long)pSymbol & 0x1) == 0x1) {
				pSymbol = pSymbol - 1;
			}
#endif
                MemCommon::setMemProt((void*)pSymbol,((unsigned long)pSymbol + nSize)-((unsigned long)pSymbol),PROT_READ | PROT_EXEC);
                if (nSize >= 2) {
#if defined(__arm__)
                    uint8_t trap[2] = { 0x01,0xde };
                    uint8_t trap2[4] = { 0xf0,0xf7,0x00,0xa0 };
                    if (memcmp(pSymbol, trap, 2) == 0 || nSize >= 4 && memcmp(pSymbol, trap2, 4) == 0) {
                        bFound = true;
                    }
#elif defined(__aarch64__)
                    uint8_t trap[4] = { 0x00,0x00,0x20,0xd4 };
                    if(nSize >= 4 && memcmp(pSymbol, trap, 4) == 0) {
                        bFound = true;
                    }
#endif
//                    if (bFound) {
//                        std::string key = STR("dg_feature3");
//                        string evidence = Common::BinToHex((unsigned char*)pSymbol, nSize);
//                        Common::addEvidence(key.c_str(), evidence.c_str());
//                    }
                }
            }
        }
        return bFound;
    }
} // Debugger_checker