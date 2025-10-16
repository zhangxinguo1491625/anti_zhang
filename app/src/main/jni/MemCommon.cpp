//
// Created by 22812 on 2025/10/2.
//

#include "MemCommon.h"
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>


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

    }
} // MemCommon