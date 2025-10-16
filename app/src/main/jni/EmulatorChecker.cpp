//
// Created by 22812 on 2025/9/16.
//

#include "EmulatorChecker.h"
#include "Common.h"
#include <vector>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>
#include <android/log.h>

namespace CEmulatorChecker {
    bool Check_Emulator(){
        if(isEmulator2()||isArm64()||GetLinkerAbi()||IsVirtualBoxBased()||libcisarm64()){
            return true;
        }
        return false;
    }
    bool isEmulator2(){
        const std::vector<std::string> EMU_FILES = {
            ("/system/bin/ldinit"),
            ("/system/bin/ldmountsf"),
            ("/system/lib/libldutils.so"),
            ("/system/bin/microvirt-prop"),
            ("/system/lib/libdroid4x.so"),
            ("/system/bin/windroyed"),
            ("/system/lib/libnemuVMprop.so"),
            ("/system/bin/microvirtd"),
            ("/system/bin/nox-prop"),
            ("/system/lib/libnoxspeedup.so"),
            ("/data/property/persist.nox.simulator_version"),
            ("/data/misc/profiles/ref/com.bignox.google.installer"),
            ("/data/misc/profiles/ref/com.bignox.app.store.hd"),
            ("/system/bin/ttVM-prop"),
            ("/system/bin/droid4x-prop"),
            ("/data/.bluestacks.prop"),
            ("/system/bin/duosconfig"),
            ("/system/etc/xxzs_prop.sh"),
            ("/system/etc/mumu-configs/device-prop-configs/mumu.config"),
            ("/data/bluestacks.prop"),
            ("/data/data/com.androVM.vmconfig"),
            ("/data/data/com.bluestacks.accelerometerui"),
            ("/data/data/com.bluestacks.appfinder"),
            ("/data/data/com.bluestacks.appmart"),
            ("/data/data/com.bluestacks.home"),
            ("/data/data/com.microvirt.market"),
            ("/dev/nemuguest"),
            ("/data/data/com.microvirt.tools"),
            ("/data/data/com.mumu.launcher"),
            ("/data/data/com.mumu.store"),
            ("/data/data/com.netease.mumu.cloner"),
            ("/system/bin/bstshutdown"),
            ("/sys/module/bstinput"),
            ("/sys/bus/pci/drivers/bstaudio"),
            ("/sys/class/misc/bst_gps"),
            ("/system/app/bluestacksHome.apk"),
            ("/system/phoenixos"),
            ("/xbin/phoenix_compat"),
            ("/init.dundi.rc"),
            ("/system/etc/init.dundi.sh"),
            ("/data/data/com.ddmnq.dundidevhelper"),
            ("/init.andy.cloud.rc"),
            ("/system/bin/xiaopiVM-prop"),
            ("/system/bin/XCPlayer-prop"),
            ("/system/lib/liblybox_prop.so"),
            ("/data/youwave_id"),
            ("/dev/vboxguest"),
            ("/dev/vboxuser"),
            ("/sys/bus/pci/drivers/vboxguest"),
            ("/sys/class/bdi/vboxsf-c"),
            ("/sys/class/misc/vboxguest"),
            ("/sys/class/misc/vboxuser"),
            ("/sys/devices/virtual/bdi/vboxsf-c"),
            ("/sys/devices/virtual/misc/vboxguest"),
            ("/sys/devices/virtual/misc/vboxuser"),
            ("/sys/module/vboxguest"),
            ("/sys/module/vboxsf"),
            ("/sys/module/vboxvideo"),
            ("/sys/module/vboxvideo/notes/.note.gnu.build-id"),
            ("/system/bin/androVM-vbox-sf"),
            ("/system/bin/androVM_setprop"),
            ("/system/bin/get_androVM_host"),
            ("/system/bin/mount.vboxsf"),
            ("/system/etc/init.androVM.sh"),
            ("/system/etc/init.buildroid.sh"),
            ("/system/lib/vboxguest.ko"),
            ("/system/lib/vboxsf.ko"),
            ("/system/lib/vboxvideo.ko"),
            ("/system/xbin/mount.vboxsf"),
            ("/dev/goldfish_pipe"),
            ("/sys/devices/virtual/misc/goldfish_pipe"),
            ("/sys/module/goldfish_audio"),
            ("/sys/module/goldfish_battery"),
            ("/sys/module/kvm_intel/"),
            ("/sys/module/kvm_amd/"),
            ("/sys/module/kvm/"),
            ("/init.android_x86_64.rc"),
            ("/init.android_x86.rc"),
            ("/init.androidVM_x86.rc"),
            ("/init.intel.rc"),
            ("/init.vbox2345_x86.rc"),
        };
        for(auto emu_file:EMU_FILES){
            if(access(emu_file.c_str(),0)==0){
                return true;
            }
        }
        const std::vector<std::string> EMU_PROPS =  {
            ("init.svc.microvirtd"),
            ("bst.version"),
            ("ro.phoenix.version.code"),
            ("ro.phoenix.version.codename"),
            ("init.svc.droid4x"),
            ("microvirt.memu_version"),
            ("microvirt.imsi"),
            ("microvirt.simserial"),
            ("ro.phoenix.version.build"),
            ("ro.phoenix.os.branch"),
            ("init.svc.su_kpbs_daemon"),
            ("init.svc.noxd"),
            ("init.svc.ttVM_x86-setup"),
            ("init.svc.xxkmsg"),
            ("ro.build.remixos.version"),
            ("microvirt.mut"),
            ("init.svc.ldinit"),
            ("ro.genymotion.version"),
            ("init.svc.pkVM_x86-setup"),
            ("ro.andy.version")
        };
        __android_log_print(ANDROID_LOG_DEBUG, "isEmulator2", "%s", Common::get_property(EMU_PROPS[0]).c_str());
        for(auto emu_prop:EMU_PROPS){
            if(!Common::get_property(emu_prop).empty()){
                __android_log_print(ANDROID_LOG_DEBUG, "isEmulator2", "JNI_OnLoad called");
                return true;
            }
        }
        return false;
    }
    bool isArm64() {
        FILE* fp = fopen("/proc/cpuinfo", "r");
        if (!fp) return false;
        char buf[256];
        while (fgets(buf, sizeof(buf), fp)) {
            if (strstr(buf, "AArch64")) {
                __android_log_print(ANDROID_LOG_DEBUG, "isEmulator3", "%s",buf);
                fclose(fp);
                return false;
            }
        }
        fclose(fp);
        return true;
    }
    bool GetLinkerAbi(){
        int fd = open("/system/bin/linker64", O_RDONLY);
        int is64;
        char buf[32];
        if (fd == -1) {
            is64 = 0;
            fd = open("/system/bin/linker", O_RDONLY);
        }
        if (fd != -1){
            read(fd, &buf, sizeof(buf));
            short* arch = (short*)(buf + 18);
            if (*arch == EM_ARM || *arch == EM_AARCH64) {

                return false;
            }
            else if (*arch == EM_386 || *arch == EM_X86_64) {
                return true;
            }
        }
        return true;
    }
    bool IsVirtualBoxBased(){
        FILE* fp = fopen("/system/lib/vboxguest.ko", "rb");
        if (fp) {
            fclose(fp);
            return true;
        }

        fp = fopen("/system/lib/vboxsf.ko", "rb");
        if (fp) {
            fclose(fp);
            return true;
        }

        return false;
    }
    bool libcisarm64() {
        unsigned long so_start = ULONG_MAX;
        unsigned long so_end = 0;
        unsigned long so_offset = 0;
        char buf[32];
        if (Common::get_map("libc.so", so_start, so_end, so_offset)) {
            __android_log_print(ANDROID_LOG_DEBUG, "libcisarm64", "libcisarm64");
            if(Common::read_remote(getpid(), buf, 32, (long) so_start) != -1){
                short* arch = (short*)(buf + 18);
                if (*arch == EM_ARM || *arch == EM_AARCH64) {
                    __android_log_print(ANDROID_LOG_DEBUG, "libcisarm64", "ARM");
                    return false;
                }
                else if (*arch == EM_386 || *arch == EM_X86_64) {
                    __android_log_print(ANDROID_LOG_DEBUG, "libcisarm64", "x86");
                    return true;
                }
            }
        }
        return false;
    }

} // CEmulatorChecker