//
// Created by 22812 on 2025/10/2.
//

#ifndef ZHANG_ANTI_MEMCOMMON_H
#define ZHANG_ANTI_MEMCOMMON_H

namespace MemCommon {
    void setMemProt(void* memory, unsigned long size, int prot);
    bool IsSoSafe();
} // MemCommon

#endif //ZHANG_ANTI_MEMCOMMON_H
