//
// Created by 抑~风 on 2023/1/31.
//

#ifndef CWJ_CO_NET_SCHEDULER_H
#define CWJ_CO_NET_SCHEDULER_H

#include <vector>
#include <list>
#include <string>
#include <cstddef>
#include <memory>
#include <atomic>

#include "thread.h"
#include "coroutine.h"
#include "log.h"

namespace CWJ_CO_NET {
    class Scheduler : public std::enable_shared_from_this<Scheduler> {
    public:

        using ptr = std::shared_ptr<Scheduler>;
        using CallBack = Coroutine::CallBack;
        using MutexType = Mutex;

        Scheduler(size_t size, bool use_cur_thread = false, std::string name = "schedule");

        virtual ~Scheduler();

        virtual void wake() = 0;

        virtual void idle() = 0;

        void stop();

        void start();

        template<typename T>
        void schedule(const T & t,int thread_id){
            CoOrFunc task(t,thread_id);
            if(task.m_co || task.m_cb){
                {
                    MutexType::Lock lock(m_mutex);
                    m_tasks.push_back(task);
                }
                wake();
            }
        }

        template<class InputIterator>
        void schedule(InputIterator begin, InputIterator end) {
            MutexType::Lock lock(m_mutex);
            bool is_wake = false;
            for(auto itr = begin;itr!=end;itr++){
                if(itr->m_cb || itr->m_co) {
                    m_tasks.push_back(*itr);
                    is_wake = true;
                }

            }
            if(is_wake)     wake();
        }

    private:

        void run();

    public:
        struct CoOrFunc{
            CoOrFunc();

            CoOrFunc(const Coroutine::ptr &mCo, int mThreadId);

            CoOrFunc(const CallBack &mCb, int mThreadId);

            void reset(){
                m_co = nullptr;
                m_cb = nullptr;
                m_thread_id = -1;
            }

            Coroutine::ptr m_co = nullptr;
            CallBack m_cb;
            int m_thread_id = -1;

        };
        static Scheduler::ptr GetThis();

        static Coroutine::ptr GetScheduleCo();

    private:
        std::vector<Thread::ptr> m_threads;
        std::list<CoOrFunc> m_tasks;

        std::string m_name;
        size_t m_thread_count = 0;
        // 下面两个指标可以作为当前调度器的忙碌指标，可用于负载均衡
        std::atomic<size_t> m_active_thread_count{0};
        std::atomic<size_t> m_idle_thread_count{0};


        // 是否将当前线程也设置为调度线程
        bool m_use_cur_thread = false;

        MutexType m_mutex;
    protected:
        std::atomic<bool> m_stopping{false};
    };
}

#endif //CWJ_CO_NET_SCHEDULER_H
