//
// Created by 抑~风 on 2023/1/25.
//

#include "log.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <utility>
#include <assert.h>

namespace CWJ_CO_NET {

    //时间戳转化为时间 毫秒级
    static std::string Stamp2Time(uint64_t timestamp, const std::string &format) {
        auto ms = timestamp % 1000;//取毫秒
        auto tick = (time_t) (timestamp / 1000);//转换时间
        struct tm t{};
        char s[40];
        t = *localtime(&tick);
        strftime(s, sizeof(s), format.c_str(), &t);
        std::string str(s);
        str = str + " " + std::to_string(ms);
        return str;
    }

    //获取系统时间戳毫秒级
    static std::string GetTimeStamp()
    {
        time_t curtime = time(NULL);
        unsigned long long time = (unsigned long long)curtime;
        return std::to_string(time);
    }


    CWJ_CO_NET::LogEvent::~LogEvent() {
        if (m_logger) {
            m_logger->log(m_level, *this);
        }
        //TODO assert(m_logger!= nullptr);
        assert(m_logger!= nullptr);
    }

    LogEvent::LogEvent(const char *mFile, int32_t mLine, uint32_t mThreadId, uint32_t mFiberId, uint64_t mTime,
                       std::string mThreadName, LogLevel mLevel, Logger::ptr logger)
            : m_file(mFile), m_line(mLine), m_threadId(mThreadId),
              m_fiberId(mFiberId), m_time(mTime), m_threadName(std::move(mThreadName)),
              m_level(mLevel), m_logger(std::move(logger)) {}

    LogLevel LogEvent::getMLevel() const {
        return m_level;
    }

    void LogEvent::setMLevel(LogLevel mLevel) {
        m_level = mLevel;
    }

    std::shared_ptr<Logger> LogEvent::getLogger() const {
        return m_logger;
    }

    void LogEvent::setLogger(std::shared_ptr<Logger> logger) {
        LogEvent::m_logger = std::move(logger);
    }

    const char *LogEvent::getMFile() const {
        return m_file;
    }

    int32_t LogEvent::getMLine() const {
        return m_line;
    }

    uint32_t LogEvent::getMThreadId() const {
        return m_threadId;
    }

    uint32_t LogEvent::getMFiberId() const {
        return m_fiberId;
    }

    uint64_t LogEvent::getMTime() const {
        return m_time;
    }

    const std::string &LogEvent::getMThreadName() const {
        return m_threadName;
    }

    void Logger::log(LogLevel level,LogEvent &event) {
        if(m_appenders.empty() && m_root)   m_root->log(level,event);
        for (const auto &a : m_appenders) {
            a->log(level, event);
        }
    }

    const std::string &Logger::getMName() const {
        return m_name;
    }

    LogLevel Logger::getMLevel() const {
        return m_level;
    }

    void Logger::addAppender(LogAppender::ptr a) {
        m_appenders.push_back(a);
    }

    void Logger::delAppender(LogAppender::ptr one) {
        for(auto  it = m_appenders.begin(); it != m_appenders.end(); it++){
            if(*it == one){
                m_appenders.erase(it);
                break;
            }
        }
    }

    void Logger::setMName(const std::string &mName) {
        m_name = mName;
    }

    Logger::Logger(const std::string &mName, LogLevel mLevel) : m_name(mName), m_level(mLevel) {}

    const Logger::ptr &Logger::getMRoot() const {
        return m_root;
    }

    void Logger::setMRoot(const Logger::ptr &mRoot) {
        m_root = mRoot;
    }

    void StdoutLogAppender::log(LogLevel level,  LogEvent &event) {
        if (m_level > event.getMLevel()) return;
        m_formatter->format(m_out, event);
    }

    StdoutLogAppender::StdoutLogAppender(const LogFormatter::ptr &mFormatter, LogLevel mLevel)
    :LogAppender(mFormatter,mLevel),m_out(std::cout){}


    void LogFormatter::format(std::ostream &os,LogEvent &event) {
//        std::cout<<"eeee"<<m_pattern << m_formatItems.size()<<std::endl;
        for (const auto &item : m_formatItems) {

            item->format(os, event);
        }
    }

    class LevelFormat: public LogFormatter::FormatItem {
    public:
        void format(std::ostream& os,LogEvent& event) override{
            os<<event.getMLevel();
        }
    };

    class FileFormat: public LogFormatter::FormatItem {
    public:
        void format(std::ostream& os,LogEvent& event) override{
            os<<event.getMFile();
        }
    };

    class LineFormat: public LogFormatter::FormatItem {
    public:
        void format(std::ostream& os,LogEvent& event) override{
            os<<event.getMLine();
        }
    };

    class TimeFormat : public LogFormatter::FormatItem {
    public:

        void setTimeFormat(const std::string &timeFormat) {
            TimeFormat::timeFormat = timeFormat;
        }

        TimeFormat(const std::string &timeFormat) : timeFormat(timeFormat) {}

        void format(std::ostream& os,LogEvent& event) override{
            os<<Stamp2Time(event.getMTime(),timeFormat);
        }
    private:
        std::string timeFormat;
    };

    class MessageFormat: public LogFormatter::FormatItem {
    public:
        void format(std::ostream& os,LogEvent& event) override{
            os<<event.getMSs().str();
        }
    };

    class LogNameFormat: public LogFormatter::FormatItem {
    public:
        void format(std::ostream& os,LogEvent& event) override{
            os<<event.getLogger()->getMName();
        }
    };

    class ThreadIdFormat: public LogFormatter::FormatItem {
    public:
        void format(std::ostream& os,LogEvent& event) override{
            os<<event.getMThreadId();
        }
    };

    class ThreadNameFormat: public LogFormatter::FormatItem {
    public:
        void format(std::ostream& os,LogEvent& event) override{
            os<<event.getMThreadName();
        }
    };

    class CoIdFormat: public LogFormatter::FormatItem {
    public:
        void format(std::ostream& os,LogEvent& event) override{
            os<<event.getMFiberId();
        }
    };


    class EndFormat: public LogFormatter::FormatItem {
    public:
        void format(std::ostream& os,LogEvent& event) override{
            os<<std::endl;
        }
    };

    class TabFormat: public LogFormatter::FormatItem {
    public:
        void format(std::ostream& os,LogEvent& event) override{
            os<<"\t";
        }
    };

    // 用于处理错误的日志格式设置
    class UnKnownFormat: public LogFormatter::FormatItem {
    public:
        void format(std::ostream& os,LogEvent& event) override{
            os<<"Unknown";
        }
    };

    class StrFormat: public LogFormatter::FormatItem {
    public:
        StrFormat(const std::string &str) : str(str) {}

        void format(std::ostream& os,LogEvent& event) override{
            os<<str;
        }

    private:
        std::string str;
    };



    LogFormatter::LogFormatter(std::string m_Pattern) : m_pattern(std::move(m_Pattern)) {

        using std::endl;
        using std::cout;
        bool hasM = false;
        bool lastTimeFormat = false;
        std::string mess;

        static std::unordered_map<char,LogFormatter::FormatItem::ptr> formatMap{
#define XX(a,b) \
         {a ,(FormatItem::ptr (new b))}\

                XX('p',LevelFormat),
                XX('F',FileFormat),
                XX('L',LineFormat),
                XX('d',TimeFormat("")),
                XX('M',MessageFormat),
                XX('C',LogNameFormat),
                XX('T',ThreadIdFormat),
                XX('N',ThreadNameFormat),
                XX('c',CoIdFormat),
                XX('n',EndFormat),
                XX('t',TabFormat),
#undef XX
        };


        for(auto i=0;i<m_pattern.size();i++){

            const auto & c = m_pattern[i];
            if(c == ' ' || c == '\n' || c == '\t')  continue;

            if(hasM){
                if(!mess.empty()){
                    this->m_formatItems.emplace_back(new StrFormat(std::move(mess)));
                    mess.clear();
                }
                if(formatMap.count(c)) {
                    this->m_formatItems.emplace_back(formatMap[c]);
                    lastTimeFormat = true;
                }
               hasM = false;
            }else if(c == '{' && lastTimeFormat){
                std::string timeFormat;
                ++i;
                for(;i<m_pattern.size() && m_pattern[i] != '}';i++){
                    timeFormat.push_back(m_pattern[i]);
                }
                if(i>= m_pattern.size()){
                    this->m_formatItems.emplace_back(new UnKnownFormat);
                }else{
                     std::shared_ptr<TimeFormat>timeItem = std::dynamic_pointer_cast<TimeFormat>(this->m_formatItems.back());
                     if(timeItem) timeItem->setTimeFormat(timeFormat);
                }
                lastTimeFormat = false;
            }else if(c == '%'){
                hasM = true;
            }
            else{
                mess.push_back(c);
            }

        }

        if(!mess.empty()){
            this->m_formatItems.emplace_back(new StrFormat(std::move(mess)));
            mess.clear();
        }

    }


    std::ostream &operator<<(std::ostream &os, const LogLevel &level) {

        switch (level){
#define XX(a,b)             \
        case a : os << #b ;  \
        break;              \

        XX(LogLevel::INFO,INFO);
        XX(LogLevel::DEBUG,DEBUG);
        XX(LogLevel::ERROR,ERROR);
        XX(LogLevel::WARN,WARN);
        XX(LogLevel::FATAL,FATAL);

#undef XX
        }
        return os;
    }

    FileLogAppender::FileLogAppender(const LogFormatter::ptr &mFormatter, LogLevel mLevel, const std::string &file)
            : LogAppender(mFormatter, mLevel), file(file),m_out(std::ofstream(file,std::ios::app)) {
    }

    void FileLogAppender::log(LogLevel level, LogEvent &event) {
        if (m_level > event.getMLevel()) return;
        if(!m_out) m_out.open(file,std::ios::app);
        assert(m_out);
        m_formatter->format(m_out, event);
    }

    LoggerManager::LoggerManager(): m_root_logger(new Logger("root",LogLevel::DEBUG)) {

        m_root_logger->addAppender(LogAppender::ptr(new StdoutLogAppender(LogFormatter::ptr(new LogFormatter),LogLevel::DEBUG)));
        m_loggers["root"] = m_root_logger;

    }

    Logger::ptr LoggerManager::getLogger(const std::string &name) {

        if(!m_loggers.count(name)) {
            m_loggers[name] = Logger::ptr(new Logger(name, LogLevel::DEBUG));
            m_loggers[name]->setMRoot(m_root_logger);
        }
        return m_loggers[name];
    }

    Logger::ptr LoggerManager::getMRootLogger() const {
        return m_root_logger;
    }
}

