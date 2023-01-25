//
// Created by 抑~风 on 2023/1/25.
//

#ifndef CWJ_CO_NET_LOG_H
#define CWJ_CO_NET_LOG_H

#include <inttypes.h>
#include "singleton.h"
#include <string>
#include <memory>
#include <list>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <fstream>
#include <string>

namespace CWJ_CO_NET{
#define CO_NET_LOG(logger,level)                                            \
      LogEvent(                                                             \
            __FILE__,__LINE__,                                              \
            1,0,                                                            \
            std::chrono::duration_cast<std::chrono::milliseconds>           \
            (std::chrono::system_clock::now().time_since_epoch()).count(),  \
            "thread_name",level,logger).getMSs()

#define DEBUG_LOG(logger) \
        CO_NET_LOG(logger,LogLevel::DEBUG)

#define INFO_LOG(logger) \
        CO_NET_LOG(logger,LogLevel::INFO)

#define WARN_LOG(logger) \
        CO_NET_LOG(logger,LogLevel::WARN)

#define ERROR_LOG(logger) \
        CO_NET_LOG(logger,LogLevel::ERROR)

#define FATAL_LOG(logger) \
        CO_NET_LOG(logger,LogLevel::FATAL)

#define GET_LOGGER(name) \
        SingleLoggerMgr::GetInstance()->getLogger(name)

#define GET_ROOT_LOGGER() \
    SingleLoggerMgr::GetInstance()->getMRootLogger()


    class Logger;


    enum class LogLevel{
            DEBUG = 1,
            INFO = 2,
            WARN = 3,
            ERROR = 4,
            FATAL = 5,

    };

    std::ostream &operator<<(std::ostream &os, const LogLevel &level);

    class LogEvent{
    public:

        LogEvent(const char *mFile, int32_t mLine,
                 uint32_t mThreadId, uint32_t mFiberId,
                 uint64_t mTime,std::string mThreadName,
                 LogLevel mLevel,std::shared_ptr<Logger> logger);

        virtual ~LogEvent();


        LogLevel getMLevel() const;

        inline void setMLevel(LogLevel mLevel);

        inline std::shared_ptr<Logger> getLogger() const;

        inline void setLogger(std::shared_ptr<Logger> logger);

        inline const char *getMFile() const;

        inline int32_t getMLine() const;

        inline uint32_t getMThreadId() const;

        inline uint32_t getMFiberId() const;

        inline uint64_t getMTime() const;

        inline const std::string &getMThreadName() const;

        std::stringstream & getMSs(){return m_ss;};

    private:
        const char* m_file = nullptr;
        int32_t  m_line = 0;
        uint32_t m_threadId = 0;
        uint32_t m_fiberId = 0;
        uint64_t m_time = 0;// 日志发生的时间
        std::string m_threadName;

        // 注意这个是用来装日志的用户内容的，而其也可以是用string,但是因为是需要重复拼接，以及支持流式操作，所以就可以用内容类的实现
        std::stringstream m_ss;
        LogLevel m_level;

        std::shared_ptr<Logger> m_logger;

    };

    /**
     *
     * %p 日志级别
     * %F 文件名
     * %L 行号
     * %d {时间格式} 日志产生时间
     * %M 消息
     * %C 日志名称
     * %T 线程号
     * %N 线程名
     * %c 协程号
     * %n 换行符
     * %t 制表符
     *
     * 默认格式 [%p] [%C] :  %d{%Y-%m-%d %H:%M:%S} %t%F %t%L %t%T %t%N %t%c %t%M;
     *
     * */

    class LogFormatter{
    public:
        using ptr = std::shared_ptr<LogFormatter>;
        void format(std::ostream& os,LogEvent& event);

        LogFormatter(std::string mPattern = "[%p]%t  %d{%Y-%m-%d %H:%M:%S} %t ==> %t [%C] %t%F %t%L %t%T %t%N %t%c %t%M");

    public:
        class FormatItem{
        public:
            using ptr = std::shared_ptr<FormatItem>;
            FormatItem() {}
            virtual ~FormatItem() {}
            virtual void format(std::ostream& os,LogEvent& event) = 0;
        };
    private:
        static std::unordered_map<char,FormatItem::ptr>formatMap;
    private:
        std::string m_pattern;
        std::list<FormatItem::ptr> m_formatItems;
    };

    class LogAppender{
    public:
        using ptr = std::shared_ptr<LogAppender>;
        LogAppender(const LogFormatter::ptr &mFormatter , LogLevel mLevel) : m_formatter(mFormatter), m_level(mLevel) {}

        virtual void log(LogLevel level,LogEvent& event) = 0;
        virtual ~LogAppender(){};

    protected:
        LogFormatter::ptr m_formatter;
        LogLevel m_level;
    };

    class Logger {
    public:
        using ptr = std::shared_ptr<Logger>;

        Logger(const std::string &mName, LogLevel mLevel);

        void log(LogLevel level,LogEvent& event);

        const std::string &getMName() const;

        LogLevel getMLevel() const;

        void addAppender(LogAppender::ptr);
        void delAppender(LogAppender::ptr);

        void setMName(const std::string &mName);

        const ptr &getMRoot() const;

        void setMRoot(const ptr &mRoot);

    private:
        std::string m_name;
        LogLevel m_level;
        std::list<LogAppender::ptr> m_appenders;
        Logger::ptr m_root;

    };


    class StdoutLogAppender : public LogAppender{
    public:
        StdoutLogAppender(const LogFormatter::ptr &mFormatter, LogLevel mLevel);

        void log(LogLevel level, LogEvent& event) override;
    private:
        std::ostream& m_out;
    };

    class FileLogAppender : public LogAppender{
    public:
        FileLogAppender(const LogFormatter::ptr &mFormatter, LogLevel mLevel, const std::string &file);

        void log(LogLevel level, LogEvent& event) override;
    private:
        std::string file;
        std::ofstream m_out;
    };

    class LoggerManager{

    public:
        using ptr = std::shared_ptr<LoggerManager>;

        LoggerManager();

        Logger::ptr getLogger(const std::string& name);

         Logger::ptr getMRootLogger() const;

    private:
        std::unordered_map<std::string,Logger::ptr>m_loggers;
        Logger::ptr m_root_logger;
    };

    using SingleLoggerMgr = Singleton<LoggerManager>;

}


#endif //CWJ_CO_NET_LOG_H
