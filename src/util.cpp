//
// Created by 抑~风 on 2023/1/29.
//
#include "util.h"
#include "cstring"
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
namespace CWJ_CO_NET{
    std::string GetAbsolutePath(const std::string & path){
        if(path.empty())    return "/";
        if(path[0] == '/'){
            return path;
        }
        static auto Deleter = [](char * c){free(c);};
        std::unique_ptr<char, decltype(Deleter)>ptPtr(get_current_dir_name(),Deleter);
        return std::string(ptPtr.get()) + "/" + path;
    }

    static bool isSameSuffix(const std::string &str, const std::string&suffix){
        if(str.size()<suffix.size())  return false;
        for(int i =suffix.size()-1,j = str.size()-1;i>=0;--i,--j){
            if(str[j] != suffix[i]){
                return false;
            }
        }
        return true;
    }

    static bool isSameSuffix(const char * str, const char* suffix){
        size_t strLen = strlen(str),sufLen = strlen(suffix);
        if(strLen<sufLen)  return false;
        auto t = str + strLen-sufLen;
        return strcmp(t,suffix) == 0;
    }

    void GetAllSameSuffixFile(std::vector<std::string>& files,const std::string & path ,const std::string& suffix){

        static auto Deleter = [](DIR * dir){closedir(dir);};
        std::unique_ptr<DIR, decltype(Deleter)>dirStream(opendir(path.c_str()),Deleter);

        struct dirent* dirInfo = nullptr;
        while((dirInfo = readdir(dirStream.get()))){
            if(dirInfo->d_type == DT_DIR){

                if(!strcmp(dirInfo->d_name,".")
                    || !strcmp(dirInfo->d_name,"..")) {
                    continue;
                }
                GetAllSameSuffixFile(files,path+"/"+std::string(dirInfo->d_name),suffix);
            }else if(dirInfo->d_type == DT_REG && isSameSuffix(dirInfo->d_name,suffix)){
                files.push_back(path+"/"+dirInfo->d_name);
            }
        }


    }

    pid_t GetThreadId() {
        return syscall(SYS_gettid);
    }

}

