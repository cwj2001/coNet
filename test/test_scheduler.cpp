//
// Created by 抑~风 on 2023/1/30.
//

#include <iostream>
#include <vector>
#include <string>
#include <condition_variable>
#include <mutex>

#include "scheduler.h"
#include "thread.h"
#include "log.h"
#include "noncopyable.h"

using namespace std;
using namespace CWJ_CO_NET;

static Logger::ptr g_logger = GET_ROOT_LOGGER();

// 实现条件变量+Scheduler实现任务管理器
class TaskScheduler : public Scheduler {
public:

    TaskScheduler(size_t size, bool useCurThread, const string &name) : Scheduler(size, useCurThread, name) {}

    virtual ~TaskScheduler() {};

    void wake() override {
        m_con.notify_one();
    }

    void idle() override {
        unique_lock<mutex> lock(m_sss);
        m_con.wait_for(lock,std::chrono::seconds(2));
        if(!this->getTaskCount()) {
            INFO_LOG(g_logger)<<"--=-=-=-=";
            m_auto_stop = true;
        }
    }

private:
    condition_variable m_con;
    mutex m_sss;
};

static void run() {
    int res = 0;
    for (int i = 0; i < 1e7; i++) {
        res += i;
    }
    INFO_LOG(g_logger) << " run finish ,res = " << res;
}

int main() {


    {
        Scheduler::ptr sc (new TaskScheduler(3, false, "scheduler1"));
        for (int i = 0; i < 200; i++) {
            sc->schedule(run, -1);
        }
        sc->start();
        sc->stop();
    }
    return 0;
}

