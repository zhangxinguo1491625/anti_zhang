//
// Created by 22812 on 2025/9/24.
//

#include "elfutil.h"
#include <elf.h>
#include <android/log.h>
namespace elfutil {
    void *global_fake_libc_base = nullptr;
    elfutil::elfutil() {
        baseAddr = NULL;
        m_nElfVer = ELF_INVALID;
        symtab_section_start_ = NULL;
        dynsym_section_start_ = NULL;
        hash_section_start_ = NULL;
        dynstr_section_start_ = NULL;
        gnu_hash_section_start_ = NULL;
        first_load_ph = NULL;
        bGnu_hash = false;
    }

    bool elfutil::loadElfFile(unsigned long startAddr) {
        if(global_fake_libc_base != nullptr && startAddr == (unsigned long)global_fake_libc_base) {
            return false;
        }
        bool bRet = false;
        uint8_t* elfBaseAddr = (uint8_t*)startAddr;

        ELF_CLASS_TYPE nELFVer = isElf(elfBaseAddr) ;
        __android_log_print(ANDROID_LOG_DEBUG, "getElfUtil", "nELFVer is  %u", nELFVer);
        if(nELFVer != ELF_INVALID) {
            this->m_nElfVer = nELFVer;
            this->baseAddr = (const uint8_t *) elfBaseAddr;
            if (this->m_nElfVer == ELF_32) {
                Elf32_Ehdr *header = (Elf32_Ehdr *) elfBaseAddr;
                Elf32_Phdr *dynamic_program_header_ = NULL;
                for (int i = 0; i < header->e_phnum; i++) {
                    Elf32_Phdr *program_header = (Elf32_Phdr *) (elfBaseAddr + header->e_phoff +
                                                                 (i * header->e_phentsize));
                    if (program_header->p_type == PT_LOAD && !first_load_ph) {
                        first_load_ph = (uint8_t *) program_header;
                    } else if (program_header->p_type == PT_DYNAMIC && !dynamic_program_header_) {
                        dynamic_program_header_ = program_header;
                    }
                    if (dynamic_program_header_) {
                        uint8_t *dsptr =
                                elfBaseAddr + (unsigned int) dynamic_program_header_->p_vaddr -
                                (unsigned int) ((Elf32_Phdr *) first_load_ph)->p_vaddr;
                        Elf32_Dyn *dynamic_section_start_ = reinterpret_cast<Elf32_Dyn *>(dsptr);
                        Elf32_Word nDynamicNum = (Elf32_Word) ((dynamic_program_header_->p_filesz) /
                                                               sizeof(Elf32_Dyn));
                        for (Elf32_Word i = 0; i < nDynamicNum; i++) {
                            Elf32_Dyn *elf_dyn = (dynamic_section_start_ + i);
                            //LOGD("%s, %d : 0x%x, offset 0x%x", __FUNCTION__, i, elf_dyn->d_tag, (unsigned char *)elf_dyn - elfBaseAddr);
                            uint8_t *d_ptr = elfBaseAddr + elf_dyn->d_un.d_ptr -
                                             (unsigned int) ((Elf32_Phdr *) first_load_ph)->p_vaddr;
                            switch (elf_dyn->d_tag) {
                                case DT_HASH:
                                    this->hash_section_start_ = d_ptr;
                                    break;
                                case 0x6ffffef5://DT_GNU_HASH:
                                    this->gnu_hash_section_start_ = d_ptr;
                                    this->bGnu_hash = true;
                                    break;
                                case DT_STRTAB:
                                    this->dynstr_section_start_ = reinterpret_cast<char *>(d_ptr);
                                    break;
                                case DT_SYMTAB:
                                    this->dynsym_section_start_ = d_ptr;
                                    break;
                            }
                        }
                        bRet = true;
                    }
                }
            }
            else {
                this->m_nElfVer = nELFVer;
                this->baseAddr = (const uint8_t *) elfBaseAddr;
                if (this->m_nElfVer == ELF_64) {
                    Elf64_Ehdr *header = (Elf64_Ehdr *) elfBaseAddr;
                    Elf64_Phdr *dynamic_program_header_ = NULL;
                    __android_log_print(ANDROID_LOG_DEBUG, "getElfUtil", "header->e_phnum is %d", header->e_phnum);
                    for (int i = 0; i < header->e_phnum; i++) {
                        Elf64_Phdr *program_header = (Elf64_Phdr *) (elfBaseAddr + header->e_phoff +
                                                                     (i * header->e_phentsize));
                        if (program_header->p_type == PT_LOAD && !first_load_ph) {
                            first_load_ph = (uint8_t *) program_header;
                        } else if (program_header->p_type == PT_DYNAMIC &&
                                   !dynamic_program_header_) {
                            dynamic_program_header_ = program_header;
                        }
                        __android_log_print(ANDROID_LOG_DEBUG, "getElfUtil", "nDynamicNum is %u", program_header->p_type);
                        if (dynamic_program_header_) {
                            uint8_t *dsptr =
                                    elfBaseAddr + (unsigned int) dynamic_program_header_->p_vaddr -
                                    (unsigned int) ((Elf64_Phdr *) first_load_ph)->p_vaddr;
                            Elf64_Dyn *dynamic_section_start_ = reinterpret_cast<Elf64_Dyn *>(dsptr);
                            Elf64_Word nDynamicNum = (Elf64_Word) (
                                    (dynamic_program_header_->p_filesz) / sizeof(Elf64_Dyn));
                            for (Elf64_Word i = 0; i < nDynamicNum; i++) {
                                Elf64_Dyn *elf_dyn = (dynamic_section_start_ + i);
                                //LOGD("%s, %d : 0x%x, offset 0x%x", __FUNCTION__, i, elf_dyn->d_tag, (unsigned char *)elf_dyn - elfBaseAddr);
                                uint8_t *d_ptr = elfBaseAddr + elf_dyn->d_un.d_ptr -
                                                 (unsigned int) ((Elf64_Phdr *) first_load_ph)->p_vaddr;
                                switch (elf_dyn->d_tag) {
                                    case DT_HASH:
                                        this->hash_section_start_ = d_ptr;
                                        break;
                                    case 0x6ffffef5://DT_GNU_HASH:
                                        this->gnu_hash_section_start_ = d_ptr;
                                        this->bGnu_hash = true;
                                        break;
                                    case DT_STRTAB:
                                        this->dynstr_section_start_ = reinterpret_cast<char *>(d_ptr);
                                        break;
                                    case DT_SYMTAB:
                                        this->dynsym_section_start_ = d_ptr;
                                        break;
                                }
                            }
                            bRet = true;
                        }
                    }
                }
            }
        }
        return bRet;
    }

    const uint8_t *
    elfutil::FindDynamicSymbolAddress(std::string &symbol_name, unsigned int *psymbol_size) {
        if(this->hash_section_start_ == NULL && this->gnu_hash_section_start_ == NULL) {
            return NULL;
        }
        if(m_nElfVer == ELF_32){
            const Elf32_Sym* sym = NULL;
            if (bGnu_hash)
                sym = FindDynamicSymbolByGnuHash<Elf32_Sym, Elf32_Off>(symbol_name.c_str());
            else
                sym = FindDynamicSymbolByHash<Elf32_Sym, Elf32_Word>(symbol_name);
            if(sym != NULL) {
                if (psymbol_size != nullptr) {
                    *psymbol_size = (unsigned int)sym->st_size;
                }
                return this->baseAddr + sym->st_value - (unsigned int)((Elf32_Phdr*)first_load_ph)->p_vaddr;
            }
        }else if(m_nElfVer == ELF_64){
            const Elf64_Sym* sym = NULL;
            if (bGnu_hash)
                sym = FindDynamicSymbolByGnuHash<Elf64_Sym, Elf64_Off>(symbol_name.c_str());
            else
                sym = FindDynamicSymbolByHash<Elf64_Sym, Elf64_Word>(symbol_name);
            if(sym != NULL) {
                if (psymbol_size != nullptr) {
                    *psymbol_size = (unsigned int)sym->st_size;
                }
                return this->baseAddr + sym->st_value - (unsigned int)((Elf64_Phdr*)first_load_ph)->p_vaddr;
            }
        }

        return NULL;
    }

    ELF_CLASS_TYPE elfutil::isElf(uint8_t* startAddr){
        if(startAddr[0] == 0x7F && (memcmp(startAddr+1, "ELF", 3) == 0)) {
            uint8_t intVer = startAddr[4];
            if(intVer == 2){
                return ELF_64;
            }else if(intVer == 1){
                return ELF_32;
            }
        }

        return ELF_INVALID;
    }
    unsigned elfutil::elfhash(const char *_name){
        const unsigned char *name = (const unsigned char *) _name;
        unsigned h = 0, g;
        while (*name) {
            h = (h << 4) + *name++;
            g = h & 0xf0000000;
            h ^= g;
            h ^= g >> 24;
        }
        return h;
    }
    uint8_t* elfutil::GetSymbolSectionStart(int section_type){
        uint8_t* sym;

        if ( section_type == SHT_SYMTAB )
        {
            sym = this->symtab_section_start_;
        }
        else if ( section_type == SHT_DYNSYM )
        {
            sym = this->dynsym_section_start_;
        }
        else
        {
            sym = nullptr;
        }
        return sym;
    }
    const char* elfutil::GetString(int section_type,int i){
        if(i > 0){
            const char* string_section_start = this->dynstr_section_start_;
            if(string_section_start) {
                return string_section_start+i;
            }
        }
        return NULL;
    }
    static inline uint32_t  dl_new_hash(const char *s)
    {
        uint32_t  h = 5381;
        for (unsigned char c = *s; c != '\0'; c = *++s)
        {
            h = h * 33 + c;
        }
        return h ;
    }

    template<class Elf_Sym, class Elf_Off>
    Elf_Sym *elfutil::FindDynamicSymbolByGnuHash(const char *sym) {
        Elf_Sym *found_sym = NULL;
        uint32_t hashval = dl_new_hash(sym);
        /* see: https://sourceware.org/ml/binutils/2006-10/msg00377.html */
        uint32_t *gnu_hash_words = (uint32_t *) gnu_hash_section_start_;
        uint32_t nbuckets = gnu_hash_words[0];
        uint32_t symbias = gnu_hash_words[1]; // only symbols at symbias up are gnu_hash'd
        uint32_t maskwords = gnu_hash_words[2]; // number of ELFCLASS-sized words in pt2 of table
        uint32_t shift2 = gnu_hash_words[3];

        Elf_Off *bloom = (Elf_Off *) &gnu_hash_words[4];
        uint32_t *buckets = (uint32_t*) (bloom + maskwords);
        uint32_t *hasharr = buckets + nbuckets;


        Elf_Off bloom_word
                = bloom[(hashval / (8*sizeof(Elf_Off)))
                        & (maskwords - 1)];

        unsigned int hash1_bitoff = hashval & (8*sizeof(Elf_Off) - 1);
        unsigned int hash2_bitoff = ((hashval >> shift2) & (8*sizeof(Elf_Off) - 1));

        if ((bloom_word >> hash1_bitoff) & 0x1
            && (bloom_word >> hash2_bitoff) & 0x1)
        {
            uint32_t lowest_symidx = buckets[hashval % nbuckets]; // might be 0
            for (uint32_t symidx = lowest_symidx;
                 symidx;
                 symidx = (!(hasharr[symidx - symbias] & 1)) ? symidx + 1 : 0)
            {
                if (((hasharr[symidx - symbias] ^ hashval) >> 1) == 0)
                {
                    Elf_Sym* symbol_start = (Elf_Sym*)GetSymbolSectionStart(SHT_DYNSYM);
                    Elf_Sym* symbol = symbol_start + symidx;
                    if (0 == strcmp(GetString(SHT_DYNSYM, symbol->st_name), sym))
                    {
                        found_sym = symbol;
                        break;
                    }
                }
            }
        }
        return found_sym;
    }

    template<class Elf_Sym, class Elf_Word>
    const Elf_Sym *elfutil::FindDynamicSymbolByHash(std::string &symbol_name) {
        Elf_Word* hash_section_start = reinterpret_cast<Elf_Word*>(this->hash_section_start_);

        Elf_Word nHashBucketNum = (hash_section_start)[0];
        if( nHashBucketNum ) {
            Elf_Word hash = elfhash(symbol_name.c_str());
            Elf_Word bucket_index = hash % nHashBucketNum;
            if (bucket_index < nHashBucketNum) {

                Elf_Word symbol_and_chain_index = hash_section_start[2+bucket_index];									//GetHashBucket()
                while (symbol_and_chain_index != 0 /* STN_UNDEF */) {

                    Elf_Sym* symbol_start = (Elf_Sym*)GetSymbolSectionStart(SHT_DYNSYM);
                    Elf_Sym* symbol = symbol_start + symbol_and_chain_index;
                    if(symbol) {
                        const char* name = GetString(SHT_DYNSYM, symbol->st_name);
                        if(name) {
                            if (symbol_name == name) {
                                return symbol;
                            }
                        }

                        symbol_and_chain_index = hash_section_start[2+nHashBucketNum+symbol_and_chain_index];			//GetHashChain()
                    }
                }
            }
        }

        return NULL;
    }


} // elfutil