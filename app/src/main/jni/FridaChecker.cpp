//
// Created by 22812 on 2025/9/22.
//

#include "FridaChecker.h"
#include "Common.h"
#include <vector>
#include <string>
#include <android/log.h>
namespace CFridaChecker {
    bool Check_Frida(){
        if(DetectFridaByThreads()){
            return true;
        }
        return false;
    }
    bool DetectFridaByThreads(){
        std::vector<std::string> task_status = Common::get_task();
        for(auto task_statu : task_status){
            __android_log_print(ANDROID_LOG_DEBUG, "DetectFridaByThreads", "%s",task_statu.c_str());
            if((strstr(task_statu.c_str(),"gum-js-loop")!= nullptr) || (strstr(task_statu.c_str(),"gmain")!= nullptr)){
                return true;
            }
        }
        return false;
    }
} // CFridaChecker