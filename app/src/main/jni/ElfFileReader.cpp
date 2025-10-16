//
// Created by 22812 on 2025/10/17.
//

#include "ElfFileReader.h"
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <android/log.h>


void ElfFileReader::getLoadedElf(const char *soname){
    FILE *fp = fopen("/proc/self/maps", "rb");
    size_t sonamelen = strlen(soname);
    char line[1024] = {0};
    unsigned long start = 0;
    unsigned long stop = 0;
    while (fgets(line, sizeof(line) - 1, fp)) {
        int pos = 0;
        char flags[4 + 1] = {'\0'};
        if (sscanf(line, "%lx-%lx %4s %*x %*x:%*x %*d%n", &start, &stop, flags, &pos) != 3) {
            continue;
        }
        if (flags[0] != 'r') {
            continue;
        }
        char *name = line + pos;
        while ((*name) == ' ') {
            ++name;
        }
        if(strstr(name, "fake-libs") != NULL){
            continue;
        }
        size_t len = strlen(name);
        name[len - 1] = '\0';
        uint32_t *magic = (uint32_t *)start;
        // 7f 45 4c 46
        if(len-1 >= sonamelen &&
           0 == strncmp(name+(len - 1)-sonamelen, soname, sonamelen) &&
           *magic == 0x464c457f){
            __android_log_print(ANDROID_LOG_DEBUG, "ElfFileReader","find so %s base %p", name, (void *)start);
            ElfW(Ehdr) *ehdr = (ElfW(Ehdr) *)start;
//            #if  defined(__i386__)
//            if(ehdr->e_machine != 3){
//				LOGD("machine type[0x%x] error, expect 0x%x\n", ehdr->e_machine, 3);
//				continue;
//			}
//            #elif  defined(__x86_64__)
//            if(ehdr->e_machine != 62){
//				LOGD("machine type[0x%x] error, expect 0x%x\n", ehdr->e_machine, 62);
//				continue;
//			}
//            #elif  defined(__arm__)
//            if(ehdr->e_machine != 40){
//				LOGD("machine type[0x%x] error, expect 0x%x\n", ehdr->e_machine, 40);
//				continue;
//			}
//            #elif  defined(__aarch64__)
//            if(ehdr->e_machine != 0xb7){
//                continue;
//            }
//            #endif
            m_fd = open(name, 0);
            m_pBase = (void *)start;
            break;
        }
    }
    fclose(fp);
}

void ElfFileReader::loadElfFile(const char *soname){
    m_fd = open(soname, 0);
    if(m_fd < 0) return;
    size_t sonamelen = strlen(soname);
    size_t sz = pread(m_fd, &m_Ehdr, sizeof(ELFW(Ehdr)), 0);
    bool is_valid_elf = true;
    ElfW(Ehdr) *ehdr = (ElfW(Ehdr) *)&m_Ehdr;

    uint32_t *magic = (uint32_t *)ehdr;
    if(*magic != 0x464c457f){
        is_valid_elf = false;
    }
    if(!is_valid_elf){
        close(m_fd);
        m_fd = -1;
    }

}
ElfFileReader::ElfFileReader(const char *filename, bool read_from_maps){
    m_fd = -1;
    m_size = 0;
    m_pBase = nullptr;
    m_pBias = nullptr;
    m_pShStrtab = nullptr;
    m_pStrtab = nullptr;
    m_pSymtab = nullptr;
    m_pShdr = nullptr;
    m_pProgramHeader = nullptr;
    m_symtabNums = 0;
    m_dynsymNums = 0;
    m_pStrDyn = nullptr;
    m_pDynsym = nullptr;
    m_pDyn = nullptr;
    if(read_from_maps){
        getLoadedElf(filename);
    }else{
        loadElfFile(filename);
    }
    if (m_fd > 0) {
        m_size = lseek(m_fd, 0, SEEK_END);
        pread(m_fd, &m_Ehdr, sizeof(ELFW(Ehdr)), 0);

        m_pShdr = (ElfW(Shdr)*)malloc(sizeof(ElfW(Shdr)) * m_Ehdr.e_shnum);
        pread(m_fd, m_pShdr, sizeof(ElfW(Shdr)) * m_Ehdr.e_shnum, m_Ehdr.e_shoff);

        ElfW(Shdr)* shstr = &(m_pShdr[m_Ehdr.e_shstrndx]);
        m_pShStrtab = (char*)malloc(shstr->sh_size);
        __android_log_print(ANDROID_LOG_DEBUG, "ElfFileReader", "read strtab from %lx, size %lx", (unsigned long)shstr->sh_offset, (unsigned long)shstr->sh_size);
        pread(m_fd, m_pShStrtab, shstr->sh_size, shstr->sh_offset);

        m_pProgramHeader = (ElfW(Phdr)*) malloc(sizeof(ElfW(Phdr)) * m_Ehdr.e_phnum);
        pread(m_fd, m_pProgramHeader, sizeof(ElfW(Phdr)) * m_Ehdr.e_phnum, m_Ehdr.e_phoff);

        parseSegment();
        parseSection();
    }
    initBias();
}
void ElfFileReader::parseSection(){
    if(m_pShStrtab == NULL || m_pShdr == NULL) return;
    for(int i = 0; i < m_Ehdr.e_shnum; i++){
        if(m_pStrtab != NULL && m_pSymtab != NULL) break;
        char *name = m_pShStrtab + m_pShdr[i].sh_name;
        //VLOGD("name:%s", name);
        if(0 == strcmp(name, ".symtab")){
            m_pSymtab = (ElfW(Sym) *)malloc(m_pShdr[i].sh_size);
            pread(m_fd, m_pSymtab, m_pShdr[i].sh_size, m_pShdr[i].sh_offset);
            m_symtabNums = m_pShdr[i].sh_size/sizeof(ElfW(Sym));
        }

        if(0 == strcmp(name, ".strtab")){
            m_pStrtab = (char*)malloc(m_pShdr[i].sh_size);
            pread(m_fd, m_pStrtab, m_pShdr[i].sh_size, m_pShdr[i].sh_offset);
        }

        if(0 == strcmp(name, ".dynamic")){
            if(m_pDyn == NULL){
                //防止内存泄漏
                m_pDyn = (ElfW(Dyn) *)malloc(m_pShdr[i].sh_size);
                pread(m_fd, m_pDyn, m_pShdr[i].sh_size, m_pShdr[i].sh_offset);
            }
        }

        if (0 == strcmp(name, ".dynsym")) {
            m_pDynsym = (ElfW(Sym)*)malloc(m_pShdr[i].sh_size);
            pread(m_fd, m_pDynsym, m_pShdr[i].sh_size, m_pShdr[i].sh_offset);
            m_dynsymNums = m_pShdr[i].sh_size / sizeof(ElfW(Sym));
        }

        if(0 == strcmp(name, ".dynstr")){
            if(m_pStrDyn == NULL){
                //防止内存泄漏
                m_pStrDyn = (char*)malloc(m_pShdr[i].sh_size);
                pread(m_fd, m_pStrDyn, m_pShdr[i].sh_size, m_pShdr[i].sh_offset);
            }
        }
    }
}
void ElfFileReader::parseSegment(){

}