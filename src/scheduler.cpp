//
// Created by 抑~风 on 2023/1/31.
//
#include "scheduler.h"
#include "coroutine.h"
#include "macro.h"
#include "log.h"
#include "copool.h"

namespace CWJ_CO_NET {

    static Logger::ptr g_logger = GET_LOGGER("system");
    static thread_local Scheduler::ptr g_scheduler = nullptr;
    static thread_local Coroutine::ptr g_scheduler_co = nullptr; // 每个线程的用于调度的主协程

    Scheduler::Scheduler(size_t size, bool use_cur_thread, std::string name,bool use_co_pool)
            : m_name(name),m_thread_count(size),m_use_cur_thread(use_cur_thread),m_use_co_pool(use_co_pool) {

        Thread::SetName(m_name + "_thread");
        if (m_use_cur_thread) m_thread_count--;

    }

    Scheduler::~Scheduler() {
        INFO_LOG(g_logger) << "Scheduler::~Scheduler()";
    }

    void Scheduler::run() {

        beforeRunScheduler();


        g_scheduler = this->shared_from_this();

        // 如果为空，就证明其是子线程，需要设置调度协程
        if (!g_scheduler_co) {
            g_scheduler_co = Coroutine::GetThis();
        }

        // TODO 注意：如果idle用一个协程来跑，然后再使用C++11条件变量来进行wait阻塞，那么会导致其无法被notify唤醒，
        // 原因目前未知
        // Coroutine::ptr idle_co(new Coroutine(std::bind(&Scheduler::idle,this),0,true));

        CoOrFunc task;
        while (true) {
            bool is_wake = false;
            bool has_task = false;
            {
                MutexType::Lock lock(m_mutex);
                if (m_stopping && m_tasks.empty()) break;

                for (auto itr = m_tasks.begin(); itr != m_tasks.end();) {
                    CWJ_ASSERT(itr->m_co || itr->m_cb);
                    if (itr->m_thread_id != -1 && itr->m_thread_id != Thread::GetPId()) {
                        is_wake = true;
                        ++itr;
                        continue;
                    }
                    if ((!itr->m_co && !itr->m_cb)
                        || (itr->m_co
                            && (itr->m_co->m_state == CoState::TERM
                                || itr->m_co->m_state == CoState::EXCEPT))) {
                        m_tasks.erase(itr++);
                        ++itr;
                        continue;
                    }
                    //TODO 判断这是否有存在的必要
                    if (itr->m_co && itr->m_co->m_state == CoState::HOLD) {
                    //     ++itr;
                    //     continue;
                    }
                    task = *itr;
                    m_tasks.erase(itr++);
                    m_active_thread_count++;
                    has_task = true;
                    break;
                }
            }

            if (is_wake || m_tasks.size()) {
                wake();
            }
            if (has_task) {
                if (!task.m_co && task.m_cb) {
                    // TODO 未测试
                    if(m_use_co_pool){
                        task.m_co = CoPoolMgr::GetInstance()->allocCo(task.m_cb, 0, true);
                    }// 下面分支已经完成测试
                    else task.m_co.reset(new Coroutine(task.m_cb, 0, true));
                }
                task.m_co->call();
                --m_active_thread_count;

                if (task.m_co->m_state == CoState::READY) {
                    schedule(task.m_co, task.m_thread_id);
                }else if(task.m_co->m_state == CoState::HOLD){

                }else if(task.m_co->m_state == CoState::TERM || task.m_co->m_state == CoState::EXCEPT){
                    if(m_use_co_pool)   {
//                        CWJ_ASSERT(false);
                        CoPoolMgr ::GetInstance()->deallocCo(task.m_co);
                    }
                }

                task.reset();

            } else if (m_stopping || m_auto_stop) {

                break;
            } else {
                ++m_idle_thread_count;
                idle();
                --m_idle_thread_count;
            }

        }

        afterRunScheduler();

    }

    void Scheduler::start() {
        // 锁加双层判断
        {
            if (m_started) return;
            MutexType::Lock lock(m_mutex);
            if (m_started) return;
            m_started = true;
        }
        m_threads.reserve(m_thread_count);
        for (int i = 0; i < m_thread_count; ++i) {
            // 注意： shared_from_this不能在构造函数被调用，因为此时还没置值
            m_threads.emplace_back(
                    new Thread(m_name+".thread_" + std::to_string(i), std::bind(&Scheduler::run, shared_from_this())));
            m_threads.back()->start();
        }
        if (m_use_cur_thread) {
            g_scheduler = shared_from_this();
            g_scheduler_co.reset(new Coroutine(std::bind(&Scheduler::run, shared_from_this()), 0, false));
            g_scheduler_co->call();
        }
        ERROR_LOG(g_logger) << " void Scheduler::start()";
    }

    Scheduler::ptr Scheduler::GetThis() {
        return g_scheduler;
    }

    Coroutine::ptr Scheduler::GetScheduleCo() {
        return g_scheduler_co;
    }

    void Scheduler::stop() {

        INFO_LOG(g_logger) << "scheduler stop";
        if(m_stopping)  return ;
        std::vector<Thread::ptr> list;
        {
            MutexType::Lock lock(m_mutex);
            if(m_stopping)  return ;
            m_stopping = true;
            list.swap(m_threads);
        }
        wakeAllThread();
        // 保证在schedule::stop函数执行后，其一定处于无任务，无线程状态
        for (auto a : list) {
            a->join();
        }
    }

    void Scheduler::wakeAllThread() {
        for (int i = 0; i < m_thread_count; i++) {
            wake();
        }
        if (m_use_cur_thread) wake();
    }

    size_t Scheduler::getTaskCount() {
        MutexType::Lock lock(m_mutex);
        return m_tasks.size();
    }

    void Scheduler::auto_stop() {
        m_auto_stop = true;
        wakeAllThread();
    }

    bool Scheduler::isStop() {
        return m_auto_stop || m_stopping;
    }

    Scheduler::CoOrFunc::CoOrFunc(const Coroutine::ptr &mCo, int mThreadId) : m_co(mCo), m_thread_id(mThreadId) {}

    Scheduler::CoOrFunc::CoOrFunc(const Scheduler::CallBack &mCb, int mThreadId) : m_cb(mCb),
                                                                                   m_thread_id(mThreadId) {}

    Scheduler::CoOrFunc::CoOrFunc() {}

}

