//
// Created by 抑~风 on 2023/1/25.
//

#include "log.h"
#include <iostream>
#include <memory>
#include <chrono>

using namespace std;
using namespace CWJ_CO_NET;

static Logger::ptr logger = GET_LOGGER("sys");

int main(){
    cout<<LogLevel::FATAL<<endl;
    cout<<LogLevel::WARN<<endl;
    cout<<LogLevel::ERROR<<endl;



    logger->addAppender(LogAppender::ptr(new FileLogAppender(std::make_shared<LogFormatter>(),LogLevel::WARN,"log.txt")));

    CO_NET_LOG(logger,LogLevel::FATAL)<<"Test macro"<<endl;
    ERROR_LOG(logger)<<"test error log"<<endl;
    INFO_LOG(logger)<<"test error log"<<endl;
    WARN_LOG(logger)<<"test error log"<<endl;
    DEBUG_LOG(logger)<<"test error log"<<endl;

}

