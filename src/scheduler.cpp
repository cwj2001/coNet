//
// Created by 抑~风 on 2023/1/31.
//
#include "scheduler.h"
#include "coroutine.h"
#include "macro.h"
#include "log.h"
#include "copool.h"
#include "config.h"

namespace CWJ_CO_NET {

    static Logger::ptr g_logger = GET_LOGGER("system");
    static thread_local Scheduler::ptr g_scheduler = nullptr;
    static thread_local Coroutine::ptr g_scheduler_co = nullptr; // 每个线程的用于调度的主协程
    static thread_local pid_t t_consume_intention_id = GetThreadId();
    static const size_t g_thread_msg_que_size = 2046;

    Scheduler::Scheduler(size_t size, bool use_cur_thread, std::string name,bool use_co_pool)
            : m_name(name),m_thread_count(size),m_use_cur_thread(use_cur_thread),m_use_co_pool(use_co_pool) {

        Thread::SetName(m_name + "_thread");
        if (m_use_cur_thread) m_thread_count--;

    }

    Scheduler::~Scheduler() {
        INFO_LOG(g_logger) << "Scheduler::~Scheduler()";
    }

    void Scheduler::run() {

        m_start_sem.wait();

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
                auto que_itr = m_thread_tasks.find(t_consume_intention_id);
                CWJ_ASSERT(que_itr != m_thread_tasks.end());
                CWJ_ASSERT(que_itr->second.get());

                auto len = m_thread_tasks.size()+1;

                for(auto i = 0;i<len;i++){
                    if(que_itr == m_thread_tasks.end()) que_itr = m_thread_tasks.begin();
                    CWJ_ASSERT(que_itr != m_thread_tasks.end());
                    CWJ_ASSERT_MGS(que_itr->second.get(),"pid:"+std::to_string(que_itr->first));

                    if (m_stopping) break;


                    {
                        // 注意：加锁
                        MutexType::Lock lock(que_itr->second->m_mutex);
                        auto &tasks = que_itr->second->m_task;
                        for (auto itr = tasks.begin(); itr != tasks.end();) {
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
                                tasks.erase(itr++);
                                ++itr;
                                continue;
                            }
                            //TODO 判断这是否有存在的必要
                            if (itr->m_co && itr->m_co->m_state == CoState::HOLD) {
                                //     ++itr;
                                //     continue;
                            }

                            task = *itr;
                            tasks.erase(itr++);

                            CWJ_ASSERT(que_itr->second->m_task_size > 0);

                            que_itr->second->m_task_size--;
                            m_active_thread_count++;
                            has_task = true;
                            break;
                        }
                    }

                    if(is_wake){
                        wake();
                    }

                    if(has_task){
                        t_consume_intention_id = que_itr->first;
                        break;
                    }

                    que_itr ++ ;

                }
            }

            if (has_task) {
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

    // 该函数是重点
    void Scheduler::start() {
        // 锁加双层判断
        {
            if (m_started) return;
            MutexType::Lock lock(m_mutex);
            if (m_started) return;


            m_threads.reserve(m_thread_count);
            for (int i = 0; i < m_thread_count; ++i) {
                // 注意： shared_from_this不能在构造函数被调用，因为此时还没置值
                m_threads.emplace_back(
                        new Thread(m_name + ".thread_" + std::to_string(i),
                                   std::bind(&Scheduler::run, shared_from_this())));
                m_threads.back()->start();
            }

            if (m_use_cur_thread) {
                g_scheduler = shared_from_this();
                g_scheduler_co.reset(new Coroutine(std::bind(&Scheduler::run, shared_from_this()), 0, false));
                // 重点：在其中会有对主线程进行一个特判，创造一个特殊的实例代表主线程
                m_threads.push_back(Thread::GetThis());
            }

            bool is_set_g_intention = false;
            // 统一分配线程的消息队列
            for(auto t : m_threads){
                m_thread_tasks[t->getMId()] = std::make_shared<Scheduler::TaskQue>();
                // 重点
                if(!is_set_g_intention) m_global_intention_id = t->getMId(),is_set_g_intention = true;
            }

            auto itr = m_thread_tasks.begin();

            for(auto one : m_share_task_que){
                CWJ_ASSERT(itr != m_thread_tasks.end());
                auto pp = itr->second;
                pp->m_task.push_back(one);
                pp->m_task_size ++ ;
                itr ++;
                if(itr == m_thread_tasks.end()) itr = m_thread_tasks.begin();
            }
            m_share_task_que.clear();


            // 线程的初始化工作都完成了，然后就可以让线程开始运行了
            for(int i=0;i<m_threads.size();i++){
                m_start_sem.notify();
            }

            wake();

            // 重点：必须在最后才能标记已经开始，以使得start仅仅被调用一次
            m_started = true;
            // 调度器必须有一个线程来处理
            CWJ_ASSERT(m_threads.size());

            if(m_use_cur_thread)   {
                g_scheduler_co->call();
            }

        }
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

//    size_t Scheduler::getTaskCount() {
//        MutexType::Lock lock(m_mutex);
//        return m_tasks.size();
//    }

    void Scheduler::auto_stop() {
        m_auto_stop = true;
        wakeAllThread();
    }

    bool Scheduler::isStop() {
        return m_auto_stop || m_stopping;
    }

    void Scheduler::SetConsumeIntentionId(int id) {
        t_consume_intention_id = id;
    }

    std::pair<pid_t ,Scheduler::TaskQue::ptr> Scheduler::checkAndGetThreadTasks( pid_t id) {
        std::pair<pid_t ,Scheduler::TaskQue::ptr> ans ;
        if(this->m_thread_tasks.count(id) == 0){
            auto insert_thread_id = m_threads[m_outside_intention_ind]->getMId();
            auto itr = m_thread_tasks.find(insert_thread_id);
            ans = {itr->first,itr->second};
            m_outside_intention_ind = (m_outside_intention_ind + 1) % m_threads.size();
        }else ans = {id,m_thread_tasks[id]};
        return ans;

    }

    Scheduler::CoOrFunc::CoOrFunc(const Coroutine::ptr &mCo, int mThreadId) : m_co(mCo), m_thread_id(mThreadId) {}

    Scheduler::CoOrFunc::CoOrFunc(const Scheduler::CallBack &mCb, int mThreadId) : m_cb(mCb),
                                                                                   m_thread_id(mThreadId) {}

    Scheduler::CoOrFunc::CoOrFunc() {}

}

