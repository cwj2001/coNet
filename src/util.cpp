//
// Created by 抑~风 on 2023/1/29.
//
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */
#include <execinfo.h>
#include <memory>
#include <iomanip>
#include <sstream>
#include <sys/prctl.h>
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

    std::string Time2Str(time_t ts, const std::string& format) {
        struct tm tm;
        localtime_r(&ts, &tm);
        char buf[64];
        strftime(buf, sizeof(buf), format.c_str(), &tm);
        return buf;
    }

    std::string StringTrim(const std::string& str, const std::string& delimit) {
        auto begin = str.find_first_not_of(delimit);
        if(begin == std::string::npos) {
            return "";
        }
        auto end = str.find_last_not_of(delimit);
        return str.substr(begin, end - begin + 1);
    }

    static const char xdigit_chars[256] = {
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,
            0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
            0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    };

    std::string UrlDecode(const std::string& str, bool space_as_plus) {
        std::string* ss = nullptr;
        const char* end = str.c_str() + str.length();
        for(const char* c = str.c_str(); c < end; ++c) {
            if(*c == '+' && space_as_plus) {
                if(!ss) {
                    ss = new std::string;
                    ss->append(str.c_str(), c - str.c_str());
                }
                ss->append(1, ' ');
            } else if(*c == '%' && (c + 2) < end
                      && isxdigit(*(c + 1)) && isxdigit(*(c + 2))){
                if(!ss) {
                    ss = new std::string;
                    ss->append(str.c_str(), c - str.c_str());
                }
                ss->append(1, (char)(xdigit_chars[(int)*(c + 1)] << 4 | xdigit_chars[(int)*(c + 2)]));
                c += 2;
            } else if(ss) {
                ss->append(1, *c);
            }
        }
        if(!ss) {
            return str;
        } else {
            std::string rt = *ss;
            delete ss;
            return rt;
        }
    }

    static const char uri_chars[256] = {
            /* 0 */
            0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1, 1, 0,
            1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 0, 0, 0, 1, 0, 0,
            /* 64 */
            0, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 0, 1,
            0, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 1, 1,
            1, 1, 1, 1, 1, 1, 1, 1,   1, 1, 1, 0, 0, 0, 1, 0,
            /* 128 */
            0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
            /* 192 */
            0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 0, 0,
    };

#define CHAR_IS_UNRESERVED(c)           \
    (uri_chars[(unsigned char)(c)])

    //-.0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz~
    std::string UrlEncode(const std::string& str, bool space_as_plus) {
        static const char *hexdigits = "0123456789ABCDEF";
        std::string* ss = nullptr;
        const char* end = str.c_str() + str.length();
        for(const char* c = str.c_str() ; c < end; ++c) {
            if(!CHAR_IS_UNRESERVED(*c)) {
                if(!ss) {
                    ss = new std::string;
                    ss->reserve(str.size() * 1.2);
                    ss->append(str.c_str(), c - str.c_str());
                }
                if(*c == ' ' && space_as_plus) {
                    ss->append(1, '+');
                } else {
                    ss->append(1, '%');
                    ss->append(1, hexdigits[(uint8_t)*c >> 4]);
                    ss->append(1, hexdigits[*c & 0xf]);
                }
            } else if(ss) {
                ss->append(1, *c);
            }
        }
        if(!ss) {
            return str;
        } else {
            std::string rt = *ss;
            delete ss;
            return rt;
        }
    }

    bool StrCmpIg(const char *a, const char *b, bool isIgnore) {
        while(*a != '\0' && *b != '\0'){
            if(isIgnore){
                if(tolower(*a) != tolower(*b))  return false;
            }else if(*a != *b)  return false;
            a += 1;
            b += 1;
        }
        if(*a != '\0' || *b != '\0')    return false;
        return true;
    }

    void SetProcessName(const std::string & name) {
        if(prctl(PR_SET_NAME, (unsigned long) name.c_str(), 0, 0, 0) == -1){
            ERROR_LOG(GET_ROOT_LOGGER()) << "Failed to set process name : "<<name;
        }
    }

    pid_t GetProcessId(){
        return syscall(SYS_getpid);
    }

    int SetNonblock(int s) {
//        int  nb;
//        nb = 1;
//        return ioctl(s, FIONBIO, &nb);

        CWJ_ASSERT(s >= 0);

        int flags=fcntl(s,F_GETFL,0);
        if(!(flags & O_NONBLOCK)) {
            flags |= O_NONBLOCK;

            if (fcntl(s, F_SETFL, flags)) {
                ERROR_LOG(GET_ROOT_LOGGER()) << "fcntl error,errno=" << errno << " strerror=" << strerror(errno);
                CWJ_ASSERT(false);
            }
        }


    }


}

