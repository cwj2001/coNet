//
// Created by 抑~风 on 2023/1/31.
//
#include "scheduler.h"
#include "coroutine.h"
#include "macro.h"
#include "log.h"

namespace CWJ_CO_NET {

    static Logger::ptr g_logger = GET_LOGGER("system");
    static thread_local Scheduler::ptr g_scheduler = nullptr;
    static thread_local Coroutine::ptr g_scheduler_co = nullptr; // 每个线程的用于调度的主协程

    Scheduler::Scheduler(size_t size, bool use_cur_thread, std::string name)
            : m_thread_count(size), m_use_cur_thread(use_cur_thread) {

        if (m_use_cur_thread) m_thread_count--;

    }

    Scheduler::~Scheduler() {

    }

    void Scheduler::run() {

        g_scheduler = this->shared_from_this();

        // 如果为空，就证明其是子线程，需要设置调度协程
        if(!g_scheduler_co){
            g_scheduler_co = Coroutine::GetThis();
        }

        Coroutine::ptr idle_co(new Coroutine(std::bind(&Scheduler::idle,this),0,true));

        bool is_wake = false;
        bool has_task = false;
        CoOrFunc task;
        while(!m_stopping){
            for(auto itr = m_tasks.begin();itr!=m_tasks.end();++itr){
                if(itr->m_thread_id != -1 && itr->m_thread_id != Thread::GetPId()){
                    is_wake = true;
                    continue;
                }
                if((!itr->m_co && !itr->m_cb)
                    || (itr->m_co
                                && (itr->m_co->m_state == CoState::TERM
                                                                    || itr->m_co->m_state == CoState::EXCEPT))){
                    m_tasks.erase(itr);
                    continue;
                }
                // TODO 判断这是否有存在的必要
                if(itr->m_co->m_state == CoState::HOLD) continue;
                task = *itr;
                m_tasks.erase(itr);
                m_active_thread_count ++;
                has_task = true;
                break;
            }

            if(is_wake || m_tasks.size()) wake();

            if(has_task){
                if(!task.m_co && task.m_cb){
                    // TODO 处理善后工作
                    task.m_co.reset(new Coroutine(task.m_cb,0,true);
                }
                task.m_co->call();
                if(task.m_co->m_state == CoState::READY){
                    schedule(task.m_co,task.m_thread_id);
                }
                -- m_active_thread_count;
                task.reset();
            }else{
                --m_active_thread_count;
                ++m_idle_thread_count;
                idle_co->call();
                --m_idle_thread_count;
                if(idle_co->m_state == CoState::TERM ){
                    WARN_LOG(g_logger)<<"idle_co TERM";
                    break;
                }else if( idle_co->m_state == CoState::EXCEPT){
                    ERROR_LOG(g_logger)<<"idle_co EXCEPT";
                    break;
                }
            }
        }


    }

    void Scheduler::start() {
        m_threads.reserve(m_thread_count);
        for (int i = 0; i < m_thread_count; ++i) {
            m_threads.emplace_back(
                    new Thread("thread_" + std::to_string(i), std::bind(&Scheduler::run, shared_from_this())));
            m_threads.back()->start();
        }
        if (m_use_cur_thread) {
            g_scheduler = shared_from_this();
            g_scheduler_co.reset(new Coroutine(std::bind(&Scheduler::run, shared_from_this()), 0, true));
            g_scheduler_co->call();
        }
    }

    Scheduler::ptr Scheduler::GetThis() {
        return g_scheduler;
    }

    Coroutine::ptr Scheduler::GetScheduleCo() {
        return g_scheduler_co;
    }

    Scheduler::CoOrFunc::CoOrFunc(const Coroutine::ptr &mCo, int mThreadId) : m_co(mCo), m_thread_id(mThreadId) {}

    Scheduler::CoOrFunc::CoOrFunc(const Scheduler::CallBack &mCb, int mThreadId) : m_cb(mCb),
                                                                                      m_thread_id(mThreadId) {}

    Scheduler::CoOrFunc::CoOrFunc() {}
}

