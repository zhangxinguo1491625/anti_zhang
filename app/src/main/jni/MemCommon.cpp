//
// Created by 22812 on 2025/10/2.
//

#include "MemCommon.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>
#include <string>
#include <dlfcn.h>
#include "EmulatorChecker.h"
#include "elfutil.h"
#include "ElfFileReader.h"
#include "android/log.h"

namespace MemCommon {
    void setMemProt(void* addr, unsigned long len, int prot)
    {
        size_t page_size = sysconf(_SC_PAGESIZE);
        uintptr_t start = (uintptr_t)addr;

        uintptr_t page_start = start & ~(page_size - 1);

        size_t len_aligned = ((start + len + page_size - 1) & ~(page_size - 1)) - page_start;

        mprotect((void*)page_start, len_aligned, prot);
        return;
    }
    bool IsSoSafe()
    {
        return true;
    }
    uint8_t* GetMethodInsnFromFile(const std::string& modulePath, const std::string& methodname, std::string& symbolName, unsigned long& symbolOff, unsigned long& symbolSize, bool isfullname) {
        uint8_t* insn = nullptr;

        ElfFileReader elffile(modulePath.c_str(), false);
        char* realSymbol = nullptr;
        ElfW(Sym) *sym = elffile.getSym(methodname.c_str(), &realSymbol, isfullname);
        if (sym != nullptr) {
            if (sym->st_value > 0 && sym->st_size > 0) {
                symbolName = realSymbol;
                symbolSize = sym->st_size;
                symbolOff = sym->st_value;

                unsigned long file_offset = elffile.getRealAddrByVaddr(sym->st_value);
                insn = new uint8_t[symbolSize];
                elffile.readContent(insn, symbolSize, file_offset);
                __android_log_print(ANDROID_LOG_DEBUG, "IsFunctionTampered","%s Symbol name:%s, address:%p offset:0x%lx size:%lu", modulePath.c_str(), realSymbol, insn, (unsigned long)symbolOff, symbolSize);
            }
        }
        return insn;
    }
    char* getPathInLine(const char* line) {
        char* path = strrchr((char*)line, ' ');
        if (path != NULL) {
            int nLen = (int)strlen(path);
            if (path[nLen - 1] == '\n') {
                path[nLen - 1] = 0;
            }

            return path + 1;
        }
        return NULL;
    }
    uint8_t* GetSoBaseInMemory(std::string& soname, std::string& fullpath)
    {
        long low = 0, high = 0;
        uint8_t* pEnd = NULL;
        FILE* fp = fopen("/proc/self/maps", "r");
        if (fp == NULL) return NULL;
        char buf[1024] = { 0 };
        while (fgets(buf, sizeof(buf) - 1, fp))
        {
            if (strstr(buf, soname.c_str()) && !strstr(buf, "fake-lib"))
            {
                sscanf(buf, "%lx-%lx", &low, &high);
                Dl_info dl_info;
                if (dladdr((void*)(low+0x120), &dl_info) == 0) {
                    //过滤非dlopen的so
                    __android_log_print(ANDROID_LOG_ERROR, "GetSoBaseInMemory","address %p not open by dlopen", (void*)low);
                    continue;
                } else {
                    __android_log_print(ANDROID_LOG_DEBUG, "GetSoBaseInMemory","%s : low %p, base %p", buf, (void*)low, (void*)dl_info.dli_fbase);
                }

                if (*(unsigned int*)low == 0x464c457f)//elf magic
                {
                    __android_log_print(ANDROID_LOG_DEBUG, "GetSoBaseInMemory","find %s base 0x%lx in line %s", soname.c_str(), low, buf);
                    char* path = getPathInLine(buf);
                    if (path != nullptr) {
                        fullpath = path;
                        __android_log_print(ANDROID_LOG_DEBUG, "GetSoBaseInMemory","module path: %s", path);
                    }

                    fclose(fp);
                    return (uint8_t*)low;
                }
            }
        }
        fclose(fp);
        return NULL;
    }
    bool IsFunctionTampered(std::string soname, std::string methodname, bool isfullname){
        bool bTampered = false;
        std::string modulePath;
        uint8_t* modulebase = GetSoBaseInMemory(soname, modulePath);
        if (modulebase != nullptr) {
            std::string symbolName;
            unsigned long symbolSize = 0;
            unsigned long symbolOff = 0;
            //从文件中读取符号指令
            uint8_t* insn = GetMethodInsnFromFile(modulePath, methodname, symbolName, symbolOff, symbolSize, isfullname);
            if (insn != nullptr && symbolOff > 0) {
                //从内存中解析符号地址
                elfutil::elfutil elfutil;
                if (!elfutil.loadElfFile((unsigned long)modulebase)) {
                    __android_log_print(ANDROID_LOG_ERROR, "IsFunctionTampered","loadElfFile failed");
                }
                unsigned long functionAddr = (unsigned long)elfutil.FindDynamicSymbolAddress(symbolName);

                if (functionAddr != 0) {
                    setMemProt((void*)functionAddr,                     \
                    functionAddr + symbolSize,    \
                    PROT_READ | PROT_EXEC);

                    unsigned long compareLength = (symbolSize < 0x10) ? symbolSize : 0x10;

//                    MemReader::printMem((uint8_t*)functionAddr, compareLength);
//                    MemReader::printMem((uint8_t*)insn, compareLength);
                    if (memcmp((void*)functionAddr, insn, compareLength) != 0) {
                        __android_log_print(ANDROID_LOG_ERROR, "IsFunctionTampered","fuction %s in %s is tampered", methodname.c_str(), soname.c_str());
                        bTampered = true;
                    } else {
                        __android_log_print(ANDROID_LOG_DEBUG, "IsFunctionTampered","fuction %s in %s is not tampered", methodname.c_str(), soname.c_str());
                    }
                }
                delete[] insn;
            }
            else {
                __android_log_print(ANDROID_LOG_ERROR, "IsFunctionTampered","get sym %s insn from module file failed", methodname.c_str());
            }
        }
        return bTampered;
    }
    bool DetectNativeHook(std::string soname, std::string methodname, bool isfullname){

        bool cpuarch = CEmulatorChecker::libcisarm64();
        bool bHooked = false;
        int index = 0;

        if(cpuarch){
            return false;
        }

        if (IsFunctionTampered(soname,methodname,isfullname)) {

            bHooked = true;
            return true;
        }

        return bHooked;
    }

} // MemCommon