//
// Created by 22812 on 2025/9/19.
//

#include "DualChecker.h"
#include "Common.h"
#include <string>
#include <vector>
#include <unistd.h>
#include "DeviceInfo.h"

namespace CDualChecker {
    bool Check_Dual(){
        if(checkVMOS()||check_uid()||CDeviceInfo::getPackagedatafile_virtual()){
            return true;
        }
        return false;
    }
    bool checkVMOS(){
        std::vector<std::string> bootlogoenvs = Common::get_env("ANDROID_BOOTLOGO");
        for(auto bootlogoenv : bootlogoenvs){
            if(strstr(bootlogoenv.c_str(),"osimg")!= nullptr || strstr(bootlogoenv.c_str(),"vmos")!= nullptr){
                return true;
            }
        }
        return false;
    }
    bool check_uid(){
        int uid = getuid();
        if (0 != uid / 100000) {
            return true;
        }
        return false;
    }
} // CBoxChecker