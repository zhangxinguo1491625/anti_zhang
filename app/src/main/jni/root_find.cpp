//
// Created by 22812 on 2025/9/11.
//
#include "root_find.h"
#include "Common.h"
#include <string>
#include <vector>
#include <unistd.h>

bool CRootChecker::Check_Root(){
    if(IsRootProperty()||IsRoot()||IsSuFilesExist()){
        return true;
    }
    return false;
}
bool CRootChecker::IsRootProperty(){
    const std::string service_adb_root = "service.adb.root";
    const std::string init_svc_adb_root = "init.svc.adb_root";
    const std::string ro_secure = "ro.secure";
    const std::string ro_debuggable = "ro.debuggable";
    if((Common::get_property("service.adb.root") == "1") ||
       (Common::get_property(init_svc_adb_root) == "running") ||
       (Common::get_property(ro_secure) == "0") ||
       (Common::get_property(ro_debuggable) == "1"))
    {
        return true;
    }else{
        return false;
    }
}
bool CRootChecker::IsRoot(){
    std::vector<std::string> rootfiles = {"/su","/magisk"};
    std::vector<std::string> pathenvs = Common::get_env("PATH");
    for(auto rootfile:rootfiles){
        for(auto pathenv:pathenvs){
            std::string all_root_file = pathenv.append(rootfile);
            if(access(all_root_file.c_str(),0)==0){
                return true;
            }
        }
    }
    return false;
}
bool CRootChecker::IsSuFilesExist(){
    const std::vector<std::string> ROOT_FILES = {
        "/su/bin/su",
        "/sbin/su",
        "/data/local/xbin/su",
        "/data/local/bin/su",
        "/data/local/su",
        "/system/xbin/su",
        "/system/bin/su",
        "/system/sd/xbin/su",
        "/system/bin/failsafe/su",
        "/system/bin/.ext/.su",
        "/system/etc/.installed_su_daemon",
        "/system/etc/.has_su_daemon",
        "/system/xbin/sugote",
        "/system/xbin/sugote-mksh",
        "/system/xbin/supolicy",
        "/system/etc/init.d/99SuperSUDaemon",
        "/system/.supersu",
        "/product/bin/su",
        "/apex/com.android.runtime/bin/su",
        "/apex/com.android.art/bin/su",
        "/system_ext/bin/su",
        "/system/xbin/bstk/su",
        "/odm/bin/su",
        "/vendor/bin/su",
        "/vendor/xbin/su",
        "/system/bin/.ext/su",
        "/system/usr/we-need-root/su",
        "/cache/su",
        "/data/su",
        "/dev/su",
    };
    for(auto rootfile:ROOT_FILES){
        if(access(rootfile.c_str(),0)==0){
            return true;
        }
    }
    return false;
}
