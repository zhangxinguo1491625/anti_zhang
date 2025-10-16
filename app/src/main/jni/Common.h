//
// Created by 22812 on 2025/9/11.
//

#ifndef ZHANG_ANTI_COMMON_H
#define ZHANG_ANTI_COMMON_H
#include <string>
#include <vector>

namespace Common {
    std::string get_property(const std::string &key);
    std::vector<std::string> get_env(const std::string &key);
    size_t read_remote(int pid, void *buf, size_t size, off_t offset);
    void split(const std::string &s, std::vector<std::string> &tokens,
                              const std::string &delimiters);
    bool get_map(const std::string &so_name,unsigned long &so_start,unsigned long &so_end,unsigned long &all_offset);
    unsigned char * get_addr(const std::string &so_name,const std::string &method_name);
    std::vector<std::string> get_task();
};

#endif //ZHANG_ANTI_COMMON_H
