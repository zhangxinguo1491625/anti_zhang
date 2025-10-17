//
// Created by 22812 on 2025/9/22.
//

#include "FridaChecker.h"
#include "Common.h"
#include "DeviceInfo.h"
#include "MemCommon.h"
#include <vector>
#include <string>
#include <android/log.h>
namespace CFridaChecker {
    bool Check_Frida(){
        if(DetectFridaByThreads()||DetectFridaByFunctionHook()){
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
    bool DetectFridaByFunctionHook(){
        int sdk_int = CDeviceInfo::device_sdk_int();
        bool Detect_bool = false;
        if(sdk_int>=19){
            std::string so_name = "libart.so";
            std::string method_name;
            if(sdk_int >= 26) {
                method_name = "_ZN3art9ArtMethod12PrettyMethodEb";
            }else{
                method_name = "_ZN3art12PrettyMethodEPNS_9ArtMethodEb";
            }
            Detect_bool = MemCommon::DetectNativeHook(so_name,method_name, true);
        }
        return Detect_bool;

    }
} // CFridaChecker