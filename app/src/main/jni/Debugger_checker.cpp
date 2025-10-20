//
// Created by 22812 on 2025/10/20.
//

#include "Debugger_checker.h"
#include <sys/uio.h>
#include <string>

namespace Debugger_checker {
    bool Check_Debugger(){
        if(isDebuggerDetectedByPort()) {
            return true;
        }
        return false
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
} // Debugger_checker