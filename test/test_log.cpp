//
// Created by 抑~风 on 2023/1/25.
//

#include <assert.h>

#include <chrono>
#include <iostream>
#include <memory>

#include "log.h"
#include "thread.h"
using namespace std;
using namespace CWJ_CO_NET;

static Logger::ptr logger = GET_LOGGER("sys");

int main() {
  assert(logger);

  cout << LogLevel::FATAL << endl;
  cout << LogLevel::WARN << endl;
  cout << LogLevel::ERROR << endl;

  fork();

  //    logger->addAppender(LogAppender::ptr(new
  //    FileLogAppender(std::make_shared<LogFormatter>(),LogLevel::WARN,"log.txt")));
  vector<Thread::ptr> list;
  for (int i = 0; i < 4; i++) {
    Thread::ptr t = make_shared<Thread>("t_" + to_string(i), []() {
      CO_NET_LOG(logger, LogLevel::FATAL) << "Test macro []" << endl;
      ERROR_LOG(logger) << "test error log []" << endl;
      INFO_LOG(logger) << "test error log" << endl;
      WARN_LOG(logger) << "test error log" << endl;
      DEBUG_LOG(logger) << "test error log []" << endl;
      // sleep(100);
    });
    t->start();
    list.push_back(t);
  }

  for (auto t : list) {
    t->join();
  }

  //    assert(false);

  CO_NET_LOG(logger, LogLevel::FATAL) << "Test macro" << endl;
  ERROR_LOG(logger) << "test error log" << endl;
  INFO_LOG(logger) << "test error log" << endl;
  WARN_LOG(logger) << "test error log" << endl;
  DEBUG_LOG(logger) << "test error log" << endl;
  //    assert(false);
  // sleep(100);
}
