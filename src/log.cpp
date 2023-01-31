//
// Created by 抑~风 on 2023/1/25.
//

#include <iostream>
#include <memory>
#include <assert.h>
#include <ctime>

#include "log.h"
#include "config.h"
#include "mutex.h"

namespace CWJ_CO_NET {



    /* ========================================= 进行全局变量的初始化 */


    static auto g_logger_list = GET_CONFIG_MGR()->
            lookup<std::vector<LoggerConfig>>("cwj_co_net.loggers"
            ,{}
            ,"the config of loggers' definition");
    static uint32_t key_update_config =  0x0;

    struct LogGVIniter{
        LogGVIniter(){

//            std::cerr<<" LogGVIniter():"<<__FILE__<<":"<<__LINE__<<std::endl;

            g_logger_list->addCallBack(key_update_config,[](const std::vector<LoggerConfig>& o
                    ,const std::vector<LoggerConfig> & n){

                for(auto& a : n){
                    auto logger = GET_LOGGER(a.m_name);
                    logger->clearAppender();
                    if(a.m_name.size())logger->setMName(a.m_name);
                    if(a.m_level.size()) logger->setMLevel(LogLevelUtil::fromString(a.m_level));
                    for(auto apr : a.m_aprs){
                        LogAppender::ptr ptr = nullptr;
                        switch (LogAppender::GetAprFromStr(apr.m_type)) {
                            case LogAprType::FILE:

                                if(apr.m_fileName.empty()){
                                    srand(time(nullptr));//srand()用于初始化随机数发生器
                                    const static std::string def_name = "cwj_co_net_log.txt";
                                    apr.m_fileName = def_name;
                                }

                                ptr.reset(
                                        new FileLogAppender(
                                                    LogFormatter::ptr(
                                                                            new LogFormatter(apr.m_format))
                                                              ,LogLevelUtil::fromString(apr.m_level)
                                                              ,apr.m_fileName));
                                break;
                            case LogAprType::STDOUT:
                                ptr.reset(new StdoutLogAppender(LogFormatter::ptr(
                                        new LogFormatter(apr.m_format))
                                        ,LogLevelUtil::fromString(apr.m_level)));
                                break;
                        }
                        if(ptr) logger->addAppender(ptr);
                    }
                }

            });
        }
    };

    static LogGVIniter gvIniter;



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
//        BacktraceToStr();
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
        // 避免在遍历的时候，另外
        RWMutex::RLock lock(m_mutex);
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
        RWMutex::WLock lock(m_mutex);
        m_appenders.push_back(a);
    }

    void Logger::delAppender(LogAppender::ptr one) {
        RWMutex::WLock lock(m_mutex);
        for(auto  it = m_appenders.begin(); it != m_appenders.end(); it++){
            if(*it == one){
                m_appenders.erase(it);
                break;
            }
        }
    }

    void Logger::setMName(const std::string &mName) {
        RWMutex::WLock lock(m_mutex);
        m_name = mName;
    }

    Logger::Logger(const std::string &mName, LogLevel mLevel) : m_name(mName), m_level(mLevel) {}

    const Logger::ptr &Logger::getMRoot() const {
        // TODO 需要加读锁
        return m_root;
    }

    void Logger::setMRoot(const Logger::ptr &mRoot) {
        RWMutex::WLock lock(m_mutex);
        m_root = mRoot;
    }

    void Logger::setMLevel(LogLevel mLevel) {
        RWMutex::WLock lock(m_mutex);
        m_level = mLevel;
    }

    void StdoutLogAppender::log(LogLevel level,  LogEvent &event) {
        if (m_level > event.getMLevel()) return;
        Mutex::Lock lock(m_mutex);
        m_formatter->format(m_out, event);
        m_out<<std::endl;
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

        if(m_Pattern.empty())   m_Pattern = GetDefaultFormat();

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

        return os<<LogLevelUtil::toString(level);
    }

    FileLogAppender::FileLogAppender(const LogFormatter::ptr &mFormatter, LogLevel mLevel, const std::string &file)
            : LogAppender(mFormatter, mLevel), file(file),m_out(std::ofstream(file,std::ios::app)) {
    }

    void FileLogAppender::log(LogLevel level, LogEvent &event) {
        if (m_level > event.getMLevel()) return;
        Mutex::Lock lock(m_mutex);
        if(!m_out) m_out.open(file,std::ios::app);
        assert(m_out);
        m_formatter->format(m_out, event);
        m_out<<std::endl;
    }

    LoggerManager::LoggerManager(): m_root_logger(new Logger("root",LogLevel::DEBUG)) {

        m_root_logger->addAppender(LogAppender::ptr(new StdoutLogAppender(LogFormatter::ptr(new LogFormatter),LogLevel::DEBUG)));
        m_loggers["root"] = m_root_logger;

    }

    Logger::ptr LoggerManager::getLogger(const std::string &name) {

        RWMutex::RLock r_lock(m_mutex);
        if(!m_loggers.count(name)) {
            r_lock.unlock();
            RWMutex::WLock w_lock(m_mutex);
            m_loggers[name] = Logger::ptr(new Logger(name, LogLevel::DEBUG));
            m_loggers[name]->setMRoot(m_root_logger);
        }
        return m_loggers[name];
    }

    Logger::ptr LoggerManager::getMRootLogger() const {
        return m_root_logger;
    }


    template<>
    class YamlCast<YAML::Node,LoggerConfig>{
    public:
        LoggerConfig operator()(const YAML::Node & source){
            assert(source.IsMap());
            LoggerConfig conf;
            if(source["level"])
                conf.m_level = YamlCast<YAML::Node,std::string>()(source["level"]);
            if(source["name"])
                conf.m_name = YamlCast<YAML::Node,std::string>()(source["name"]);
            if(source["appenders"])
                conf.m_aprs = std::move(YamlCast<YAML::Node,std::vector<LogAprConfig>>()(source["appenders"]));
            return conf;
        }
    };

    template<>
    class YamlCast<YAML::Node,LogAprConfig>{
    public:
        LogAprConfig operator()(const YAML::Node & source){
            assert(source.IsMap());
            LogAprConfig conf;
            if(source["level"])
                conf.m_level = YamlCast<YAML::Node,std::string>()(source["level"]);
            if(source["format"])
                conf.m_format = YamlCast<YAML::Node,std::string>()(source["format"]);
            if(source["type"])
                conf.m_type = YamlCast<YAML::Node,std::string>()(source["type"]);
            if(source["file"])
                conf.m_fileName = YamlCast<YAML::Node,std::string>()(source["file"]);
            return conf;
        }
    };

    template<>
    class YamlCast<LoggerConfig,YAML::Node>{
    public:
        YAML::Node operator()(const LoggerConfig & source){
            YAML::Node root(YAML::NodeType::Map);
            root["level"] = YamlCast<std::string,YAML::Node>()(source.m_level);
            root["name"] = YamlCast<std::string,YAML::Node>()(source.m_name);
            root["appenders"] = YamlCast<std::vector<LogAprConfig>,YAML::Node>()(source.m_aprs);
            return root;
        }
    };

    template<>
    class YamlCast<LogAprConfig,YAML::Node>{
    public:
        YAML::Node operator()(const LogAprConfig & source){

            YAML::Node root(YAML::NodeType::Map);
            root["level"] = YamlCast<std::string,YAML::Node>()(source.m_level);
            root["format"] = YamlCast<std::string,YAML::Node>()(source.m_format);
            root["type"] = YamlCast<std::string,YAML::Node>()(source.m_type);

            return root;
        }
    };


     LogAprType LogAppender::GetAprFromStr(const std::string &type) {
         if(type == "file"){
             return LogAprType::FILE;
         }else if(type == "stdout"){
             return LogAprType::STDOUT;
         }
        return LogAprType::UNKNOW;
    }

    std::string LogLevelUtil::toString(const LogLevel &level) {
         std::string res ;
        switch (level){
#define XX(a,b)             \
        case a : res =  #b ;  \
        break;              \

            XX(LogLevel::INFO,INFO);
            XX(LogLevel::DEBUG,DEBUG);
            XX(LogLevel::ERROR,ERROR);
            XX(LogLevel::WARN,WARN);
            XX(LogLevel::FATAL,FATAL);
            XX(LogLevel::UNKNOW,UNKNOW);
#undef XX
        }
        return std::move(res);
    }

    LogLevel LogLevelUtil::fromString(const std::string &str) {

#define XX(a,b) \
        if(str == #b){ \
            return a; \
        }

        XX(LogLevel::INFO,INFO);
        XX(LogLevel::DEBUG,DEBUG);
        XX(LogLevel::ERROR,ERROR);
        XX(LogLevel::WARN,WARN);
        XX(LogLevel::FATAL,FATAL);
#undef XX
        return LogLevel::UNKNOW;
    }

    bool LogAprConfig::operator==(const LogAprConfig &rhs) const {
        return m_type == rhs.m_type &&
               m_format == rhs.m_format &&
               m_level == rhs.m_level &&
               m_fileName == rhs.m_fileName;
    }

    bool LogAprConfig::operator!=(const LogAprConfig &rhs) const {
        return !(rhs == *this);
    }

    bool LoggerConfig::operator==(const LoggerConfig &rhs) const {
        return m_aprs == rhs.m_aprs &&
               m_level == rhs.m_level &&
               m_name == rhs.m_name;
    }

    bool LoggerConfig::operator!=(const LoggerConfig &rhs) const {
        return !(rhs == *this);
    }
}

