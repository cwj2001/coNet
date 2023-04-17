//
// Created by 抑~风 on 2023/1/28.
//

#ifndef CWJ_CO_NET_UTIL_H
#define CWJ_CO_NET_UTIL_H
#include <cxxabi.h>
#include <unistd.h>
#include <memory>
#include <filesystem>
#include <vector>
#include <sys/types.h>
#include <dirent.h>
#include <chrono>
#include <sys/socket.h>
#include <sys/ioctl.h>


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

    int SetNonblock(int s);


    void SetProcessName(const std::string & name);

    void Backtrace(std::vector<std::string>&trace,int size,int skip);

    std::string BacktraceToStr(int size = 256,int skip = 0,const std::string& prefix = "",const std::string& suffix = "\n");

    uint64_t GetCurrentMs();

    std::string Time2Str(time_t ts, const std::string& format);

    std::string StringTrim(const std::string& str, const std::string& delimit);

    std::string UrlDecode(const std::string& str, bool space_as_plus);

    std::string UrlEncode(const std::string& str, bool space_as_plus) ;

    bool StrCmpIg(const char * a,const char *b,bool isIgnore);

//    template <typename T>
//    struct TimeData {
//        double count;
//        T data;
//        friend std::ostream &operator<<(std::ostream &os, const TimeData &res) {
//            os << "res: " << res.data << " time: " << res.count << "ms" ;
//            return os;
//        }
//    };
//    template<typename T,typename A,typename... Args>
//    auto CalcTime(T func,TimeData<A>data,Args... rest) -> decltype(data){
//        auto start = std::chrono::system_clock::now();
//        data.data = func(rest...);
//        auto end = std::chrono::system_clock::now();
//        std::chrono::duration<double> diff = end - start;
//        data.count = diff.count();
//        return data;
//    }



    template<typename T,typename... Args>
    auto CalcTime(T&& func, Args&&... rest) -> double{
        auto start = std::chrono::duration_cast<std::chrono::milliseconds>
                        (std::chrono::system_clock::now().time_since_epoch()).count();
        func(std::forward<Args>(rest)...);
        auto end = std::chrono::duration_cast<std::chrono::milliseconds>
                        (std::chrono::system_clock::now().time_since_epoch()).count();
//    std::chrono::duration<double> diff = end-start;
        // 计算毫秒时间差并输出
        // 如果要求其他时间单位可以修改 std::chrono::milliseconds 为其他类型
        // 比如std::chrono::seconds
        return end-start;
    }

    pid_t GetProcessId();


}

#endif //CWJ_CO_NET_UTIL_H
