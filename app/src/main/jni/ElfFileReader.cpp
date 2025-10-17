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
ElfFileReader::~ElfFileReader(){
    if(m_fd > 0){
        close(m_fd);
    }

    if(m_pStrtab){
        free(m_pStrtab);
    }

    if(m_pSymtab){
        free(m_pSymtab);
    }

    if(m_pShdr){
        free(m_pShdr);
    }

    if(m_pProgramHeader){
        free(m_pProgramHeader);
    }

    if (m_pDynsym) {
        free(m_pDynsym);
    }

    if (m_pStrDyn) {
        free(m_pStrDyn);
    }

    if (m_pDyn) {
        free(m_pDyn);
    }

    if (m_pShStrtab) {
        free(m_pShStrtab);
    }
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
    if(m_pProgramHeader == NULL) return;

    for(int i = 0; i < m_Ehdr.e_phnum; i++){
        if(m_pProgramHeader[i].p_type == PT_DYNAMIC){
            m_pDyn = (ElfW(Dyn) *)malloc(m_pProgramHeader[i].p_filesz);
            pread(m_fd, m_pDyn, m_pProgramHeader[i].p_filesz, m_pProgramHeader[i].p_offset);
            int dynamic_size= m_pProgramHeader[i].p_filesz / sizeof(ElfW(Dyn));
            int strtab_size_=0;
            unsigned long  offset=0;
            for(int j=0;j<dynamic_size;j++){
                if(m_pDyn[j].d_tag==DT_STRSZ){
                    strtab_size_ = m_pDyn[j].d_un.d_val;
                }
                else if(m_pDyn[j].d_tag == DT_STRTAB){
                    offset=m_pDyn[j].d_un.d_ptr;
                    offset=getRealAddrByVaddr(offset);
                }
            }
            if(strtab_size_!=0 && offset!=0){
                m_pStrDyn = (char*)malloc(strtab_size_);
                pread(m_fd, m_pStrDyn, strtab_size_, offset);
            }
        }
    }
}
bool ElfFileReader::getSymBySymtab(const char *symname, void **result, int minsize){
    __android_log_print(ANDROID_LOG_DEBUG, "ElfFileReader","get symbol %s, m_pStrtab %p, m_pSymtab %p", symname, m_pStrtab, m_pSymtab);
    if(m_pStrtab == NULL || m_pSymtab == NULL){
        return false;
    }

    for(int i = 0 ; i < m_symtabNums; i++){
        ElfW(Sym) *sym = &(m_pSymtab[i]);
        char *name = m_pStrtab + sym->st_name;
        //LOGD("compare name %s with %s",name, symname);
        if(0 == strcmp(name, symname) && sym->st_size >= minsize){
            __android_log_print(ANDROID_LOG_DEBUG, "ElfFileReader","find name %s", symname);
            *result = (void *)( (unsigned long)(sym->st_value)  + (unsigned long)m_pBias);
            return true;
        }

    }
    return false;
}
unsigned long ElfFileReader::getRealAddrByVaddr(unsigned long vaddr){
    for (size_t i = 0; i < m_Ehdr.e_phnum; i++) {
        ElfW(Phdr) *phdr = &m_pProgramHeader[i];
        if (phdr->p_vaddr <= vaddr && (phdr->p_vaddr + phdr->p_memsz) > vaddr) {
            return phdr->p_offset + (vaddr - phdr->p_vaddr);
        }
    }
    return 0;
}

ElfW(Sym) * ElfFileReader::getSym(const char *symname, char** realSymbol, bool bCompareWhole, void* func){
    __android_log_print(ANDROID_LOG_DEBUG, "ElfFileReader","get symbol %s, m_pSymtab %p(%d), m_pDynsym %p(%d)", symname, m_pSymtab, m_symtabNums, m_pDynsym, m_dynsymNums);
    if(m_pStrtab != NULL && m_pSymtab != NULL){
        for(int i = 0 ; i < m_symtabNums; i++){
            ElfW(Sym) *sym = &(m_pSymtab[i]);
            char *name = m_pStrtab + sym->st_name;
            //LOGD("[%d/%d] : compare name %s with %s", i, m_symtabNums, name, symname);
            if(0 == strcmp(name, symname) || (!bCompareWhole && strstr(name, symname))){
                if (sym->st_value == 0) {
                    continue;
                }
                __android_log_print(ANDROID_LOG_DEBUG, "ElfFileReader","find name %s from .symtab", name);
                if (realSymbol != NULL) {
                    *realSymbol = name;
                }

                if(func == nullptr){
                    return sym;
                }else{
                    if(((HookCallbackFunc)func)(getModuleBias(), sym, name)){
                        return sym;
                    }
                }
            }

        }
    }

    if (m_pStrDyn != NULL && m_pDynsym != NULL) {
        for (int i = 0; i < m_dynsymNums; i++) {
            ElfW(Sym)* sym = &(m_pDynsym[i]);
            char* name = m_pStrDyn + sym->st_name;
            //LOGD("[%d/%d] : compare name %s with %s", i, m_dynsymNums, name, symname);
            if (0 == strcmp(name, symname) || (!bCompareWhole && strstr(name, symname))) {
                if (sym->st_value == 0) {
                    continue;
                }
                __android_log_print(ANDROID_LOG_DEBUG, "ElfFileReader","find name %s from .dynsym", name);
                if (realSymbol != NULL) {
                    *realSymbol = name;
                }

                if(func == nullptr){
                    return sym;
                }else{
                    if(((HookCallbackFunc)func)(getModuleBias(), sym, name)){
                        return sym;
                    }
                }
            }

        }
    }
    return NULL;
}
size_t ElfFileReader::readContent(uint8_t *buffer, size_t size, off_t offset){
    return pread(m_fd, buffer, size, offset);
}
//void *getSymByDynSym();
void ElfFileReader::initBias(){
    ElfW(Addr) min_vaddr = UINTPTR_MAX;
    if(m_pBase != NULL){
        for(int i = 0; i < m_Ehdr.e_phnum; i++){
            if(m_pProgramHeader[i].p_type == PT_LOAD){
                min_vaddr = min_vaddr > m_pProgramHeader[i].p_vaddr ? m_pProgramHeader[i].p_vaddr : min_vaddr;
            }
        }
        m_pBias = (void *)(((ElfW(Addr))m_pBase) - min_vaddr);
    }
}

bool ElfFileReader::getNeededSos(std::vector<std::string> &needsos){
    if(m_pStrDyn == NULL || m_pDyn == NULL){
        return false;
    }

    for(ElfW(Dyn) *d = m_pDyn; d->d_tag != 0; d++){
        if(d->d_tag == DT_NEEDED){
            char *name = m_pStrDyn + d->d_un.d_val;
            needsos.push_back(name);
        }
    }
    return true;
}
void* ElfFileReader::getTextSectionInfo(unsigned int& offset, unsigned int& size){
    void* pText = NULL;
    if(m_pShStrtab != NULL && m_pShdr != NULL){
        for(int i = 0; i < m_Ehdr.e_shnum; i++){
            char *name = m_pShStrtab + m_pShdr[i].sh_name;
            if(0 == strcmp(name, ".text")) {
                pText = malloc(m_pShdr[i].sh_size);
                pread(m_fd, pText, m_pShdr[i].sh_size, m_pShdr[i].sh_offset);
                offset = m_pShdr[i].sh_offset;
                size = m_pShdr[i].sh_size;
                __android_log_print(ANDROID_LOG_DEBUG, "ElfFileReader","text section offset:%08x sh_size:%08x",offset, size);
            }
        }
    }

    return pText;
}
