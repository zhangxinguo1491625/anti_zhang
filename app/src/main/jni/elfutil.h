//
// Created by 22812 on 2025/9/24.
//

#ifndef ZHANG_ANTI_ELFUTIL_H
#define ZHANG_ANTI_ELFUTIL_H

#include <cstdint>
#include <string>

namespace elfutil {
    typedef enum {
        ELF_INVALID = 0,
        ELF_32,
        ELF_64,
    }ELF_CLASS_TYPE;

    class elfutil {
        public:
            elfutil();
            bool loadElfFile(unsigned long startAddr);
            const uint8_t* FindDynamicSymbolAddress(std::string& symbol_name, unsigned int* psymbol_size = nullptr);
        protected:
            ELF_CLASS_TYPE isElf(uint8_t* startAddr);
            static unsigned elfhash(const char *_name);
            uint8_t* GetSymbolSectionStart(int section_type);
            const char* GetString(int section_type,int i);

            template<class Elf_Sym, class Elf_Word> const Elf_Sym* FindDynamicSymbolByHash(std::string& symbol_name);
            template<class Elf_Sym, class Elf_Off> Elf_Sym *FindDynamicSymbolByGnuHash(const char *sym);

        private:
            const uint8_t* baseAddr;
            uint8_t* symtab_section_start_;
            uint8_t* dynsym_section_start_;
            uint8_t* hash_section_start_;
            char* dynstr_section_start_;
            uint8_t *gnu_hash_section_start_;
            uint8_t *first_load_ph;
            bool bGnu_hash;
            ELF_CLASS_TYPE m_nElfVer;
    };

} // elfutil

#endif //ZHANG_ANTI_ELFUTIL_H
