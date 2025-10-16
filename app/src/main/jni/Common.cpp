//
// Created by 22812 on 2025/9/11.
//

#include "Common.h"
#include <string>
#include <vector>

#include <sys/system_properties.h>
#include <sys/uio.h>
#include <android/log.h>
#include <dirent.h>
#include <elfutil.h>
#define PROP_VALUE_MAX  92

namespace Common {
    std::string get_property(const std::string &key) {
        if (key.empty()) return "";
        char buf[PROP_VALUE_MAX]{};
        __system_property_get(key.c_str(), buf);
        return {buf};
    }

    std::vector<std::string> get_env(const std::string &key) {
        std::vector<std::string> paths{};
        if (key.empty()) return paths;

        char *envPaths = getenv(key.c_str());
        if (envPaths != nullptr && strlen(envPaths) > 0) {
            Common::split(envPaths, paths, ":");
            return paths;
        }
        return paths;
    }
    bool get_map(const std::string &so_name,unsigned long &so_start,unsigned long &so_end,unsigned long &all_offset){
        FILE* fp = fopen("/proc/self/maps", "rb");
        if (!fp) return false;
        char line[1024] = { 0 };
        unsigned long start;
        unsigned long end;
        unsigned long offset;
        bool so_find = false;
        while (fgets(line, sizeof(line) - 1, fp)) {
            int pos;
            char flags[5] = {'\0'};
            if (sscanf(line, "%lx-%lx %4s %lx %*x:%*x %*d%n", &start, &end, flags, &offset, &pos) != 4) {
                __android_log_print(ANDROID_LOG_DEBUG, "get_map", "read /proc/self/maps wrong");
                continue;
            }
            if(strstr(line,so_name.c_str())!=nullptr){
                if(so_start > start){
                    so_start = start;
                }
                else if(so_end < end){
                    so_end = end;
                }
                all_offset = offset + end - start;
                so_find = true;
            }
        }
        if(so_find) {
            return true;
        }
        return false;

    }
    size_t read_remote(int pid, void *buf, size_t size, off_t offset){

        struct iovec local[1];
        local[0].iov_base = buf;
        local[0].iov_len = size;

        struct iovec remote[1];
        remote[0].iov_base = reinterpret_cast<void *>(offset);
        remote[0].iov_len = size;

        // 读取目标进程内存
        ssize_t nread = process_vm_readv(pid, local, 1, remote, 1, 0);

        if (nread == -1) {
            __android_log_print(ANDROID_LOG_DEBUG, "read_remote", "read_remote wrong");
        }
        return nread;
    }
    void split(const std::string &s, std::vector<std::string> &tokens,
                              const std::string &delimiters) {
        std::string::size_type lastPos = s.find_first_not_of(delimiters, 0);
        std::string::size_type pos = s.find_first_of(delimiters, lastPos);
        while (std::string::npos != pos || std::string::npos != lastPos) {
            tokens.push_back(s.substr(lastPos, pos - lastPos));
            lastPos = s.find_first_not_of(delimiters, pos);
            pos = s.find_first_of(delimiters, lastPos);
        }
    }
    std::vector<std::string> get_task(){
        std::vector<std::string> status{};
        DIR* dir = opendir("/proc/self/task");
        if (dir == nullptr) {
            perror("opendir");
            return status;
        }
        struct dirent* entry;
        char filename[260] = { 0 };
        while ((entry = readdir(dir)) != NULL) {
            // 跳过 "." 和 ".."
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            snprintf(filename, sizeof(filename) - 1, "/proc/self/task/%s/status", entry->d_name);
            FILE* fp = fopen(filename, "r");
            if (!fp) continue;
            char line[1024] = { 0 };
            if (fgets(line, sizeof(line) - 1, fp) != NULL)
            {
                std::string prefix = "Name:";
                std::string str_line(line);
                size_t pos = str_line.find(prefix);
                if (pos != std::string::npos) {
                    // 去掉 "Name:" 和前后的空格
                    std::string name = str_line.substr(pos + prefix.length());
                    name.erase(0, name.find_first_not_of(" \t")); // 左 trim
                    name.erase(name.find_last_not_of(" \t") + 1); // 右 trim4
                    status.push_back(name);
                }
            }
        }
        closedir(dir);
        return status;
    }

    elfutil::elfutil * getElfUtil(const std::string soname )
    {
        elfutil::elfutil * pElfUtil = NULL;

        unsigned long so_start = ULONG_MAX;
        unsigned long so_end = 0;
        unsigned long so_offset = 0;
        char buf[32];
        if (Common::get_map(soname.c_str(), so_start, so_end, so_offset)){
            unsigned long pSoMemBase = so_start;
            pElfUtil = new elfutil::elfutil();
            if (pElfUtil->loadElfFile(pSoMemBase) == true){
                __android_log_print(ANDROID_LOG_DEBUG, "getElfUtil","%s loadElfFile success,insert soname:%s pElfUtil:%p pSoMemBase:%p", __FUNCTION__,soname.c_str(), pElfUtil, pSoMemBase);
            }else{
                __android_log_print(ANDROID_LOG_DEBUG, "getElfUtil","%s loadElfFile failed", __FUNCTION__);
                delete pElfUtil;
                pElfUtil = NULL;
            }
        }else{
            __android_log_print(ANDROID_LOG_DEBUG, "getElfUtil","%s get %s base from /proc/self/maps failed", __FUNCTION__,soname.c_str());
        }

        return pElfUtil;
    }

    unsigned char * get_addr(const std::string &so_name,const std::string &method_name)
    {
        unsigned char *pSymbol = NULL;

        elfutil::elfutil * pElfUtil = getElfUtil(so_name);
        if(pElfUtil){
            std::string strTemp = method_name;
            pSymbol = (unsigned char*)pElfUtil->FindDynamicSymbolAddress(strTemp);
        }
        return pSymbol;
    }
}
