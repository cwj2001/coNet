//
// Created by 抑~风 on 2023-04-15.
//

#ifndef CWJ_CO_NET_PROCESS_H
#define CWJ_CO_NET_PROCESS_H

#include <memory>

#include <sys/types.h>
#include <unistd.h>
#include <functional>
#include <vector>
//#include "iomanager.h"

#include "cwj_channel.h"

namespace CWJ_CO_NET{

    class Process{
    public:
        using ptr = std::shared_ptr<Process>;
        using socket_t = int;
        using CallBack = std::function<void()>;
        enum class State{
            EXEC,
            EXIT,
            TERM,
            INIT,
        };

        static int GetProcessId();

        static void UpdateProcessId();

    public:
        pid_t m_pid = -1;
        State m_status = State::INIT;
        socket_t m_channel[2];
        CallBack m_cb;
        std::string m_name;
        // 目前没有用，方便扩展
        void *data;

    };



}

/*
 * 对于管理进程
 * 1. 信号处理
 * 2. 初始化主进程的管理结构
 * 3. 启动work线程
 * 4. 管理进程工作循环
 *
 * */


#endif //CWJ_CO_NET_PROCESS_H
