//
// Created by 22812 on 2025/10/17.
//

#ifndef ZHANG_ANTI_ELFFILEREADER_H
#define ZHANG_ANTI_ELFFILEREADER_H
#include <elf.h>
#include <vector>
#include <string>

#if defined(__LP64__)
#define ElfW(type) Elf64_ ## type
#else
#define ElfW(type) Elf32_ ## type
#endif


#if defined(__LP64__)
#define ELFW(what) Elf64_ ## what
#else
#define ELFW(what) Elf32_ ## what
#endif


typedef bool (*HookCallbackFunc)(void* base, ElfW(Sym)* sym, char* realSymbol);
class ElfFileReader {
    public:
        ElfFileReader(const char *filename, bool read_from_maps = true);
        ~ElfFileReader();
        void getLoadedElf(const char *filename);
        void loadElfFile(const char *filename);
        void parseSection();
        void parseSegment();
        bool getSymBySymtab(const char *symname, void **result, int minsize);
        unsigned long getRealAddrByVaddr(unsigned long vaddr);

        ElfW(Sym) * getSym(const char *symname, char** realSymbol = NULL, bool bCompareWhole = true, void* func = nullptr);
        size_t readContent(uint8_t *buffer, size_t size, off_t offset);
        void* getModuleBase(){return m_pBase;};
        void* getModuleBias(){return m_pBias;};
        //void *getSymByDynSym();
        void initBias();

        bool getNeededSos(std::vector<std::string> &needsos);
        void* getTextSectionInfo(unsigned int& offset, unsigned int& size);
    private:
        int m_fd;
        size_t m_size;
        ElfW(Ehdr) m_Ehdr;
        void *m_pBase;     //base是内存里elf的起始地址
        void *m_pBias;     //这个才是真正的基址
        char *m_pShStrtab;
        char * m_pStrtab;
        ElfW(Sym) *m_pSymtab;
        ElfW(Sym)* m_pDynsym;
        ElfW(Shdr) *m_pShdr;
        ElfW(Phdr) *m_pProgramHeader;
        int m_symtabNums;
        int m_dynsymNums;

        //add dynamic info
        char *m_pStrDyn;
        ElfW(Dyn) *m_pDyn;

};
#endif //ZHANG_ANTI_ELFFILEREADER_H
