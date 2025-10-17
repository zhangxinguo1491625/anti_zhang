//
// Created by 22812 on 2025/10/2.
//

#ifndef ZHANG_ANTI_MEMCOMMON_H
#define ZHANG_ANTI_MEMCOMMON_H
#include <string>

namespace MemCommon {
    void setMemProt(void* memory, unsigned long size, int prot);
    bool IsSoSafe();
    uint8_t* GetMethodInsnFromFile(const std::string& modulePath, const std::string& methodname, std::string& symbolName, unsigned long& symbolOff, unsigned long& symbolSize, bool isfullname);
    char* getPathInLine(const char* line);
    uint8_t* GetSoBaseInMemory(std::string& soname, std::string& fullpath);
    bool IsFunctionTampered(std::string soname, std::string methodname, bool isfullname);
    bool DetectNativeHook(std::string soname, std::string methodname, bool isfullname);
} // MemCommon

#endif //ZHANG_ANTI_MEMCOMMON_H
