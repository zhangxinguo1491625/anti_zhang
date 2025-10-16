//
// Created by 22812 on 2025/10/9.
//

#include "DeviceInfo.h"
#include "Common.h"
#include <map>
#include <string>
#include <jni.h>
#include <unistd.h>
#include <android/log.h>
#include "Common_Sensor.h"
#include <sys/stat.h>

namespace CDeviceInfo {
    jstring Check_DeviceInfo(JNIEnv *env) {
        std::map<std::string, std::string> device_map;
        std::string restring = "{";
        std::string android_id_str("android_id");
        device_map["android_id"] = get_Secure_id(env, android_id_str);
        std::string adb_enable_str("adb_enabled");
        device_map["adb_enabled"] = get_Global_id(env, adb_enable_str);
        device_map["fingerprint"] = get_fingerprint(env);
        device_map["serial"] = get_serial(env);
        device_map["ssh"] = get_ssh();
        device_map["bootload"] = get_bootload();
        device_map["httpproxy"] = get_proxy();
        device_map["vpn"] = get_vpn(env);
        device_map["network_list"] = get_network_list(env);
        std::string accessibility_enabled_str("accessibility_enabled");
        device_map["accessibility_enabled"] = get_Secure_id(env,accessibility_enabled_str);
        device_map["accessibility_package"] = enabled_accessibility_package(env);
        device_map["sensor_list"] = CCommon_Sensor::get_sensor();
        device_map["get_sim_property"] = get_sim_property();
        device_map["packagename"] = get_package(env);
        device_map["getPackagedatafile"] = getPackagePath(env);

        device_map["getPackagedatafile"] = getPackagePath(env);
        for (auto &p: device_map) {
            restring = restring + p.first + ":" + p.second + ";";
        }
        restring = restring + "}";
        return env->NewStringUTF(restring.c_str());
    }

    std::string get_Secure_id(JNIEnv *env, std::string &key) {
        jclass cls = env->FindClass("android/provider/Settings$Secure");
        jmethodID methodid = env->GetStaticMethodID(cls, "getString",
                                                    "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;");
        jstring key_str = env->NewStringUTF(key.c_str());
        jclass contextCls = env->GetObjectClass(globalContext);
        jmethodID mid_getResolver = env->GetMethodID(contextCls,
                                                     "getContentResolver",
                                                     "()Landroid/content/ContentResolver;");
        jobject resolver = env->CallObjectMethod(globalContext, mid_getResolver);

        jstring result = (jstring) env->CallStaticObjectMethod(cls, methodid, resolver, key_str);
        if (result != nullptr) {
            const char *msg = env->GetStringUTFChars(result, nullptr);
            std::string str_line(msg);
            return str_line;
        } else {
            return nullptr;
        }
    }
    std::string get_Global_id(JNIEnv *env, std::string &key) {
        jclass cls = env->FindClass("android/provider/Settings$Global");
        jmethodID methodid = env->GetStaticMethodID(cls, "getString",
                                                    "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;");
        jstring key_str = env->NewStringUTF(key.c_str());
        jclass contextCls = env->GetObjectClass(globalContext);
        jmethodID mid_getResolver = env->GetMethodID(contextCls,
                                                     "getContentResolver",
                                                     "()Landroid/content/ContentResolver;");
        jobject resolver = env->CallObjectMethod(globalContext, mid_getResolver);

        jstring result = (jstring) env->CallStaticObjectMethod(cls, methodid, resolver, key_str);
        if (result != nullptr) {
            const char *msg = env->GetStringUTFChars(result, nullptr);
            std::string str_line(msg);
            return str_line;
        } else {
            return nullptr;
        }
    }
    std::string get_fingerprint(JNIEnv *env) {
        jclass cls = env->FindClass("android/os/Build");
        jfieldID fid = env->GetStaticFieldID(cls, "FINGERPRINT", "Ljava/lang/String;");
        jobject obj = env->GetStaticObjectField(cls, fid);
        // 注意：这里返回的是 jobject，需要强转为 jstring
        jstring jstr = static_cast<jstring>(obj);
        const char *msg = env->GetStringUTFChars(jstr, nullptr);
        std::string str_line(msg);
        return str_line;
    }
    std::string get_serial(JNIEnv *env) {
        if(sdk_int(env)<26) {
            std::string serial = Common::get_property("ro.serialno");
            if (serial.empty()) {
                serial = Common::get_property("ro.boot.serialno");
            }
            return serial;
        }else{
            jclass version_cls = env->FindClass("android/os/Build");
            jmethodID getSerial_method = env->GetStaticMethodID(version_cls,"getSerial","()Ljava/lang/String;");
            jstring getSerial = (jstring)env->CallStaticObjectMethod(version_cls,getSerial_method);
            if (env->ExceptionCheck()) {
                // 捕获异常
                env->ExceptionDescribe();   // 输出异常信息到 logcat
                env->ExceptionClear();      // 清除 pending exception
                return "";
            }
            const char *msg = env->GetStringUTFChars(getSerial, nullptr);
            std::string getSerial_str(msg);
            return getSerial_str;
        }
    }
    std::string get_ssh() {
        std::vector<std::string> pathenvs = Common::get_env("PATH");
        for(auto pathenv:pathenvs){
            std::string all_root_file = pathenv.append("/scp");
            if(access(all_root_file.c_str(),0)==0){
                return "scp access";
            }
        }
        if(!Common::get_property("init.svc.dropbear_key").empty()){
            return "scp property has";
        }
        return "no ssh";
    }
    std::string get_bootload(){
        std::vector<std::string> pathenvs = {
                "/sdcard/TWRP",
                "/sdcard/Download/magisk_patched.img",
                "/system/vendor/bin/install-recovery.sh",
                "/system/vendor/etc/install-recovery.sh",
                "/system/vendor/etc/recovery-resource.dat",
                "/vendor/bin/install-recovery.sh",
                "/vendor/etc/install-recovery.sh",
                "/vendor/etc/recovery-resource.dat"
        };
        std::vector<std::string> propvects = {"ro.boot.verifiedbootstate",
                                             "ro.boot.flash.locked",
                                             "ro.boot.veritymode",
                                             "ro.boot.vbmeta.device_state"};

        std::string res_prop;
        for(auto pathenv:pathenvs){
            if(access(pathenv.c_str(),0)==0){
                res_prop.append(pathenv);
                res_prop.append(" access|");
            }
        }
        for(auto propvect:propvects){
            res_prop.append(Common::get_property(propvect));
        }
        return res_prop;
    }
    std::string get_vpn(JNIEnv* env){
        jclass NetworkInterface_cls = env->FindClass("java/net/NetworkInterface");
        jmethodID getNetworkInterfaces_methodid = env->GetStaticMethodID(NetworkInterface_cls, "getNetworkInterfaces",
                                                    "()Ljava/util/Enumeration;");
        jobject NetworkInterface_Enumeration = env->CallStaticObjectMethod(NetworkInterface_cls,getNetworkInterfaces_methodid);
        if(NetworkInterface_Enumeration == nullptr){
            return "";
        }
        jclass Collections_cls = env->FindClass("java/util/Collections");
        jmethodID list_methodid = env->GetStaticMethodID(Collections_cls,"list","(Ljava/util/Enumeration;)Ljava/util/ArrayList;");
        jobject NetworkInterface_list = env->CallStaticObjectMethod(Collections_cls,list_methodid,NetworkInterface_Enumeration);
        int i = 0;
        jclass list_cls = env->GetObjectClass(NetworkInterface_list);
        jmethodID list_size_methodid = env->GetMethodID(list_cls,"size","()I");
        jmethodID list_get_methodid = env->GetMethodID(list_cls,"get","(I)Ljava/lang/Object;");
        jint eth_len = env->CallIntMethod(NetworkInterface_list,list_size_methodid);
        for(int i = 0;i<eth_len;i++){
            jobject NetworkInterface = env->CallObjectMethod(NetworkInterface_list,list_get_methodid,i);
            jclass NetworkInterface_cls = env->GetObjectClass(NetworkInterface);
            jmethodID isup_methodid = env->GetMethodID(NetworkInterface_cls,"isUp","()Z");
            jboolean isup_res = env->CallBooleanMethod(NetworkInterface,isup_methodid);
            if(isup_res){
                jmethodID getName_methodid = env->GetMethodID(NetworkInterface_cls,"getName","()Ljava/lang/String;");
                jstring jname =  static_cast<jstring>(env->CallObjectMethod(NetworkInterface,getName_methodid));
                if (jname) {
                    const char *msg = env->GetStringUTFChars(jname, nullptr);
                    std::string str_line(msg);
                    if(str_line.find("tun") != std::string::npos || str_line.find("ppp") != std::string::npos){
                        jmethodID getInetAddresses_methodid = env->GetMethodID(NetworkInterface_cls,"getInetAddresses",
                                                                                     "()Ljava/util/Enumeration;");
                        jobject InetAddresses_Enumeration = env->CallObjectMethod(NetworkInterface,getInetAddresses_methodid);
                        jobject InetAddresses_list = env->CallStaticObjectMethod(Collections_cls,list_methodid,InetAddresses_Enumeration);
                        jint ip_len = env->CallIntMethod(InetAddresses_list,list_size_methodid);
                        for(int j = 0;j<ip_len;j++) {
                            jobject InetAddresses = env->CallObjectMethod(InetAddresses_list,list_get_methodid,j);
                            jclass InetAddresses_cls = env->GetObjectClass(InetAddresses);
                            jmethodID isLoopbackAddress_methodid = env->GetMethodID(InetAddresses_cls,"isLoopbackAddress",
                                                                                    "()Z;");
                            jboolean isLoopbackAddress_res = env->CallBooleanMethod(InetAddresses,isLoopbackAddress_methodid);
                            if(!isLoopbackAddress_res){
                                jmethodID getHostAddress_methodid = env->GetMethodID(InetAddresses_cls,"getHostAddress",
                                                                                        "()Ljava/lang/String;");
                                jstring getHostAddress_res = static_cast<jstring>(env->CallObjectMethod(InetAddresses,getHostAddress_methodid));
                                const char* getHostAddress_chars = env->GetStringUTFChars(getHostAddress_res, nullptr);
                                std::string getHostAddress_result(getHostAddress_chars);
                                return str_line + "->" + getHostAddress_result;
                            }

                        }
                        return str_line;
                    }
                }
            }
        }
        return "";
    }
    std::string get_network_list(JNIEnv* env){
        jclass NetworkInterface_cls = env->FindClass("java/net/NetworkInterface");
        jmethodID getNetworkInterfaces_methodid = env->GetStaticMethodID(NetworkInterface_cls, "getNetworkInterfaces",
                                                                         "()Ljava/util/Enumeration;");
        jobject NetworkInterface_Enumeration = env->CallStaticObjectMethod(NetworkInterface_cls,getNetworkInterfaces_methodid);
        if(NetworkInterface_Enumeration == nullptr){
            return "";
        }
        jclass Collections_cls = env->FindClass("java/util/Collections");
        jmethodID list_methodid = env->GetStaticMethodID(Collections_cls,"list","(Ljava/util/Enumeration;)Ljava/util/ArrayList;");
        jobject NetworkInterface_list = env->CallStaticObjectMethod(Collections_cls,list_methodid,NetworkInterface_Enumeration);
        int i = 0;
        jclass list_cls = env->GetObjectClass(NetworkInterface_list);
        jmethodID list_size_methodid = env->GetMethodID(list_cls,"size","()I");
        jmethodID list_get_methodid = env->GetMethodID(list_cls,"get","(I)Ljava/lang/Object;");
        jint eth_len = env->CallIntMethod(NetworkInterface_list,list_size_methodid);
        for(int i = 0;i<eth_len;i++) {
            jobject NetworkInterface = env->CallObjectMethod(NetworkInterface_list,
                                                             list_get_methodid, i);
            jclass NetworkInterface_cls = env->GetObjectClass(NetworkInterface);
            jmethodID isup_methodid = env->GetMethodID(NetworkInterface_cls, "isUp", "()Z");
            jboolean isup_res = env->CallBooleanMethod(NetworkInterface, isup_methodid);
            jmethodID isLoopback_methodid = env->GetMethodID(NetworkInterface_cls, "isLoopback", "()Z");
            jboolean isLoopback_res = env->CallBooleanMethod(NetworkInterface, isLoopback_methodid);
            jmethodID isVirtual_methodid = env->GetMethodID(NetworkInterface_cls, "isVirtual", "()Z");
            jboolean isVirtual_res = env->CallBooleanMethod(NetworkInterface, isVirtual_methodid);
            if(isup_res && !isLoopback_res && !isVirtual_res) {
                jmethodID getName_methodid = env->GetMethodID(NetworkInterface_cls,"getName","()Ljava/lang/String;");
                jstring jname =  static_cast<jstring>(env->CallObjectMethod(NetworkInterface,getName_methodid));
                if (jname) {
                    const char *msg = env->GetStringUTFChars(jname, nullptr);
                    std::string str_line(msg);
                    if(str_line.find("wlan")!=std::string::npos || str_line.find("eth")!=std::string::npos){
                        jmethodID getInetAddresses_methodid = env->GetMethodID(NetworkInterface_cls,"getInetAddresses",
                                                                               "()Ljava/util/Enumeration;");
                        jobject InetAddresses_Enumeration = env->CallObjectMethod(NetworkInterface,getInetAddresses_methodid);
                        jobject InetAddresses_list = env->CallStaticObjectMethod(Collections_cls,list_methodid,InetAddresses_Enumeration);
                        jint ip_len = env->CallIntMethod(InetAddresses_list,list_size_methodid);
                        if (env->ExceptionCheck()) {
                            env->ExceptionDescribe();
                            env->ExceptionClear();
                            return "";
                        }
                        //__android_log_print(ANDROID_LOG_DEBUG, "get_network_list", "size -> %d",ip_len);
                        for(int j = 0;j<ip_len;j++) {
                            jobject InetAddresses = env->CallObjectMethod(InetAddresses_list,list_get_methodid,j);
                            if (env->ExceptionCheck()) {
                                env->ExceptionDescribe();
                                env->ExceptionClear();
                                return "";
                            }
                            //jclass InetAddresses_cls = env->GetObjectClass(InetAddresses);
                            jclass InetAddresses_cls = env->FindClass("java/net/InetAddress");
                            jmethodID isLoopbackAddress_methodid = env->GetMethodID(InetAddresses_cls,"isLoopbackAddress",
                                                                                    "()Z");
                            jboolean isLoopbackAddress_res = env->CallBooleanMethod(InetAddresses,isLoopbackAddress_methodid);
                            if(!isLoopbackAddress_res){
                                jmethodID getHostAddress_methodid = env->GetMethodID(InetAddresses_cls,"getHostAddress",
                                                                                     "()Ljava/lang/String;");
                                jstring getHostAddress_res = static_cast<jstring>(env->CallObjectMethod(InetAddresses,getHostAddress_methodid));
                                const char* getHostAddress_chars = env->GetStringUTFChars(getHostAddress_res, nullptr);
                                std::string getHostAddress_result(getHostAddress_chars);
                                return str_line + "->" + getHostAddress_result;
                            }

                        }
                    }
                }
            }
        }
        return "";
    }
    std::string enabled_accessibility_package(JNIEnv* env){
        jclass contextCls = env->GetObjectClass(globalContext);
        jmethodID getSystemService_methodid = env->GetMethodID(contextCls,
                                                     "getSystemService",
                                                     "(Ljava/lang/String;)Ljava/lang/Object;");
        jstring accessibility_str = env->NewStringUTF("accessibility");
        jobject AccessibilityManager_res = env->CallObjectMethod(globalContext,getSystemService_methodid,accessibility_str);
        jclass AccessibilityManager_Cls = env->GetObjectClass(AccessibilityManager_res);
        jmethodID getEnabledAccessibilityServiceList_methodid = env->GetMethodID(AccessibilityManager_Cls,"getEnabledAccessibilityServiceList",
                                                                                 "(I)Ljava/util/List;");
        jobject AccessibilityService_List = env->CallObjectMethod(AccessibilityManager_res,getEnabledAccessibilityServiceList_methodid,-1);
        jclass list_cls = env->GetObjectClass(AccessibilityService_List);
        jmethodID list_size_methodid = env->GetMethodID(list_cls,"size","()I");
        jmethodID list_get_methodid = env->GetMethodID(list_cls,"get","(I)Ljava/lang/Object;");
        jint accessibilityService_list_size = env->CallIntMethod(AccessibilityService_List,list_size_methodid);
        std::string sb;
        for(int i = 0;i<accessibilityService_list_size;i++){
            jobject accessibilityService_object = env->CallObjectMethod(AccessibilityService_List,list_get_methodid,i);
            jclass accessibilityService_cls = env->GetObjectClass(accessibilityService_object);
            jmethodID getResolveInfo = env->GetMethodID(accessibilityService_cls,"getResolveInfo","()Landroid/content/pm/ResolveInfo");
            jobject ResolveInfo_object = env->CallObjectMethod(accessibilityService_object,getResolveInfo);
            jclass ResolveInfo_cls = env->GetObjectClass(ResolveInfo_object);
            jfieldID serviceInfo_field = env->GetFieldID(ResolveInfo_cls,"serviceInfo","Landroid/content/pm/ServiceInfo");
            jobject serviceInfo_object = env->GetObjectField(ResolveInfo_object,serviceInfo_field);
            jclass serviceInfo_cls = env->GetObjectClass(serviceInfo_object);
            jfieldID ApplicationInfo_field = env->GetFieldID(serviceInfo_cls,"applicationInfo","Landroid/content/pm/ApplicationInfo");
            jobject ApplicationInfo_object = env->GetObjectField(serviceInfo_object,ApplicationInfo_field);
            jclass ApplicationInfo_cls = env->GetObjectClass(ApplicationInfo_object);
            jfieldID flags_field = env->GetFieldID(ApplicationInfo_cls,"flags","I");
            jint flags = env->GetIntField(ApplicationInfo_object,flags_field);
            if(((flags&1) ==0) && ((flags&128) ==0)){
                jfieldID packageName_field = env->GetFieldID(serviceInfo_cls,"packageName","Ljava/lang/String;");
                jfieldID name_field = env->GetFieldID(serviceInfo_cls,"name","Ljava/lang/String;");
                jstring packageName_jstr = static_cast<jstring>(env->GetObjectField(serviceInfo_object,
                                                                               packageName_field));
                jstring name_jstr = static_cast<jstring>(env->GetObjectField(serviceInfo_object,
                                                                        name_field));
                const char *packageName_chr = env->GetStringUTFChars(packageName_jstr, nullptr);
                std::string packageName_line(packageName_chr);
                const char *name_chr = env->GetStringUTFChars(name_jstr, nullptr);
                std::string name_line(name_chr);
                sb = packageName_line + "/" + name_line + ":";
            }

        }
        return sb;
    }
    std::string get_proxy(){
        return Common::get_property("http.proxyHost") + ":" + Common::get_property("http.proxyPort");
    }
    jint sdk_int(JNIEnv* env){
        jclass version_cls = env->FindClass("android/os/Build$VERSION");
        jfieldID sdk_int_field = env->GetStaticFieldID(version_cls,"SDK_INT","I");
        jint sdk_int = env->GetStaticIntField(version_cls,sdk_int_field);
        return sdk_int;
    }
    std::string get_sim_property(){
        std::vector<std::string> propvects = {"gsm.operator.alpha",
                                              "gsm.operator.iso-country",
                                              "gsm.operator.numeric",
                                              "gsm.sim.operator.alpha",
                                              "gsm.sim.operator.iso-country",
                                              "gsm.sim.operator.numeric",
                                              "gsm.network.type",
                                              "gsm.sim.state"};
        std::string res_prop;
        for(auto propvect:propvects){
            res_prop.append(Common::get_property(propvect) + ",");
        }
        return res_prop;
    }
    std::string get_package(JNIEnv* env){
        jclass contextCls = env->GetObjectClass(globalContext);

        // 2. 获取包名
        jmethodID getPackageName_methodid = env->GetMethodID(contextCls, "getPackageName", "()Ljava/lang/String;");
        jstring packageName_jstr = (jstring)env->CallObjectMethod(globalContext, getPackageName_methodid);
        const char *packageName_chr = env->GetStringUTFChars(packageName_jstr, nullptr);
        std::string packageName_line(packageName_chr);
        return packageName_line;
    }
    std::string getPackagePath(JNIEnv* env){
        if(env == nullptr){
            __android_log_print(ANDROID_LOG_DEBUG, "getPackagePath", "wrong");
            return "";
        }
        jclass contextCls = env->FindClass("android/content/Context");
        jmethodID getApplicationInfo_methodid = env->GetMethodID(contextCls, "getApplicationInfo", "()Landroid/content/pm/ApplicationInfo;");
        jobject applicationInfo = env->CallObjectMethod(globalContext, getApplicationInfo_methodid);
        jclass applicationInfo_cls = env->GetObjectClass(applicationInfo);
        jfieldID sourceDir_field = env->GetFieldID(applicationInfo_cls,"sourceDir","Ljava/lang/String;");
        jstring sourceDir_jstr = static_cast<jstring>(env->GetObjectField(applicationInfo,sourceDir_field));
        const char *sourceDir_chr = env->GetStringUTFChars(sourceDir_jstr, nullptr);
        std::string sourceDir_line(sourceDir_chr);
        return sourceDir_line;
    }
    std::string getPackagedatafile(JNIEnv* env){
        jclass contextCls = env->FindClass("android/content/Context");
        jmethodID getApplicationInfo_methodid = env->GetMethodID(contextCls, "getApplicationInfo", "()Landroid/content/pm/ApplicationInfo;");
        jobject applicationInfo = env->CallObjectMethod(globalContext, getApplicationInfo_methodid);
        jclass applicationInfo_cls = env->GetObjectClass(applicationInfo);
        jfieldID dataDir_field = env->GetFieldID(applicationInfo_cls,"dataDir","Ljava/lang/String;");
        jstring dataDir_jstr = static_cast<jstring>(env->GetObjectField(applicationInfo,dataDir_field));
        const char *daraDir_chr = env->GetStringUTFChars(dataDir_jstr, nullptr);
        std::string dataDir_line(daraDir_chr);
        return dataDir_line;
    }
    bool getPackagedatafile_virtual() {
        int uid = getuid();
        std::string getpackage_str = getPackagedatafile(g_env);
        struct stat st {};
        int ret = stat(getpackage_str.c_str(), &st);
        jint data_file_uid = -1;
        if (ret == 0) {
            data_file_uid = st.st_uid; // ← 获取 UID
        }
        if (data_file_uid == uid){
            return false;
        }
        std::string getpackage_data_str = "/data/data/";
        getpackage_data_str.append(get_package(g_env));
        ret = stat(getpackage_data_str.c_str(), &st);
        if (ret == 0) {
            data_file_uid = st.st_uid; // ← 获取 UID
        }
        if (data_file_uid == uid){
            return false;
        }
        return true;
    }
}