//
// Created by 抑~风 on 2023/1/29.
//
#include <cstring>
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <execinfo.h>
#include <memory>
#include <iomanip>
#include <sstream>

#include "util.h"
#include "log.h"
#include "macro.h"

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
        if(!dirStream) {
            INFO_LOG(GET_ROOT_LOGGER()) <<"path:"<< path.c_str() << " not exist";
            CWJ_ASSERT(dirStream);
        }
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

    static std::string traceToStr(const char * trace){
//        static char tmp [256];
        size_t size = 0;
        int status = 0;
        std::string rt;
        rt.resize(256);
        if(1 == sscanf(trace, "%*[^(]%*[^_]%255[^)+]",&rt[0])) {
            char* v = abi::__cxa_demangle(&rt[0], nullptr, &size, &status);
            if(v) {
                std::string result(v);
                free(v);
                return result;
            }
        }
        if(1 == sscanf(trace, "%255s", &rt[0])) {
            return rt;
        }
        return trace;
    }

    void Backtrace(std::vector<std::string>&traces,int size,int skip){

        static auto Deleter = [](const auto &a){free(a);};
        using DeleterType = decltype(Deleter);

        auto buf = std::unique_ptr<void*,DeleterType>((void**)malloc(size),Deleter);
        int n = backtrace(buf.get(),size);
        auto trace_sym = std::unique_ptr<char*,DeleterType>(backtrace_symbols(buf.get(),n),Deleter);

        if(!trace_sym){
            ERROR_LOG(GET_LOGGER("system"))<<"Backtrace backtrace_symbols errors ";
        }
        for(int i=skip;i<n;i++){
            traces.emplace_back(traceToStr(trace_sym.get()[i]));
        }
        return ;
    }

    std::string BacktraceToStr(int size, int skip, const std::string &prefix,const std::string& suffix) {
        std::cout<<"==="<<std::endl;
        std::vector<std::string>list;
        Backtrace(list,size,skip);
        std::stringstream ss;
        for(int i=0;i<list.size();i++){
            ss<<prefix<<" ["<<std::setw(2)<<std::setfill('0')<<i<<"] "<<list[i]<<suffix;
        }
        return ss.str();
    }

    uint64_t GetCurrentMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>           \
            (std::chrono::system_clock::now().time_since_epoch()).count();
    }


}

