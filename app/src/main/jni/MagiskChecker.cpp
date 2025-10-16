//
// Created by 22812 on 2025/9/19.
//

#include "MagiskChecker.h"
#include "Common.h"
#include <vector>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>
#include <android/log.h>

namespace CMagiskChecker {
    bool Check_Magisk(){
        if(CheckZygisk()||FindMagiskFiles()||CheckNativebridge()){
            __android_log_print(ANDROID_LOG_DEBUG, "Check_Magisk", "%d,%d",CheckZygisk(),CheckNativebridge());
            return true;
        }
        return false;
    }
    bool FindMagiskFiles(){
        FILE* fp = fopen("/proc/self/mounts", "r");
        if (!fp) return false;
        char line[1024] = { 0 };
        while (fgets(line, sizeof(line) - 1, fp)) {
            if(strstr(line, "magisk") != NULL){
                return true;
            }
        }
        return false;
    }
    bool CheckZygisk(){
        FILE* fp = fopen("/proc/self/attr/prev", "r");
        if (!fp) return false;
        char line[1024] = { 0 };
        while (fgets(line, sizeof(line) - 1, fp)) {
            if(strstr(line,"u:r:zygote:s0")!=nullptr)
            {
                return true;
            }
        }
        return false;
    }

    bool CheckNativebridge(){
        //NativeBridgeError
        bool (*NativeBridgeError)() = (bool (*)())Common::get_addr("libnativebridge.so","NativeBridgeError");
        if(NativeBridgeError != NULL){
            __android_log_print(ANDROID_LOG_DEBUG, "CheckNativebridge","CheckNativebridge is %d", NativeBridgeError());
            if(NativeBridgeError()){
                return true;
            }
        }
        return false;
    }
} // CMagiskChecker