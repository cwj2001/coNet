//
// Created by 抑~风 on 2023/1/28.
//

#ifndef CWJ_CO_NET_UTIL_H
#define CWJ_CO_NET_UTIL_H
#include <cxxabi.h>
#include <unistd.h>
#include "log.h"
#include <memory>
#include <filesystem>
#include <vector>
#include <sys/types.h>
#include <dirent.h>

namespace CWJ_CO_NET{

 static const int PATH_SIZE = 255;

    template<class T>
    const char* TypeToName() {
        static const char* s_name = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
        return s_name;
    }

    std::string GetAbsolutePath(const std::string & path);

    void GetAllSameSuffixFile(std::vector<std::string>& files,const std::string & path ,const std::string& suffix);

    pid_t GetThreadId();

}

#endif //CWJ_CO_NET_UTIL_H
