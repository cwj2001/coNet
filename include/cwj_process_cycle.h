//
// Created by 抑~风 on 2023-04-15.
//

#ifndef CWJ_CO_NET_CWJ_PROCESS_CYCLE_H
#define CWJ_CO_NET_CWJ_PROCESS_CYCLE_H

#include <memory>

#include <sys/types.h>
#include <unistd.h>
#include <functional>
#include <vector>
#include "iomanager.h"

#include "cwj_signal.h"
#include "socket.h"
#include "cwj_channel.h"

namespace CWJ_CO_NET{
    class MasterProcess{
    public:

        using ptr = std::shared_ptr<MasterProcess>;

        MasterProcess(size_t worker_size = 1,size_t worker_pre_thread_num = 1);
        using CalllBack = std::function<void(IOManager::ptr)>;
        void start_process_cycle();

        void setMWorkerCycle(const CalllBack &mWorkerCycle);

        void setMMasterCycle(const CalllBack &mMasterCycle);

    private:
        std::vector<SignalCtx> m_signal_ctx_master;
        std::vector<SignalCtx> m_signal_ctx_worker;
        std::string m_master_name = "Master";
        std::string m_worker_name = "Worker";
        uint32_t m_worker_pre_thread_num = 1;
        uint32_t m_worker_size = 1;
        CalllBack m_worker_cycle;
        CalllBack m_master_cycle;
        // 父节点通信事件，accept事件，普通处理事件
        IOManager::ptr m_iom;

        bool init_signals();
        bool init_master_process();
        bool start_work_processes();
        uint32_t spawn_process(std::basic_string<char, std::char_traits<char>, std::allocator<char>> basicString, void *i);
        void handleChannel();
        void workCycle();
        void passOpenChannel(uint32_t slot,const CWJ_CO_NET::Channel &channel);
        void initWorkCycle();



        int reclaimChildren(int pid);

        void spawnWorker(int slot);


        void signalWorkerProcesses(int signo);

        void midWorkerCycle();
    };

}

#endif //CWJ_CO_NET_CWJ_PROCESS_CYCLE_H
