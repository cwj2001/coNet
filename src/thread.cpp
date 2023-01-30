//
// Created by 抑~风 on 2023/1/30.
//

#include "thread.h"
#include "log.h"

namespace CWJ_CO_NET{

    static auto g_logger = GET_LOGGER("system");
    static thread_local Thread::ptr g_cur_thread  = nullptr;
    static thread_local std::string g_cur_thread_name = "UNKNOW" ;

    Thread::Thread(const std::string &mName, const Thread::CallBack &cb)
                                                : m_name(mName)
                                                , m_cb(cb)
                                                , m_sem(0){
    }

    Thread::~Thread() {
        WARN_LOG(g_logger)<<"~Thread";
        if(m_thread) {
            pthread_detach(m_thread);
            m_thread = 0;
        }
        g_cur_thread.reset();
        g_cur_thread_name = "UNKNOW";
    }

    pid_t Thread::getMId() const {
        return m_id;
    }

    const std::string &Thread::getMName() const {
        return m_name;
    }

    void Thread::join() {
        ERROR_LOG(g_logger)<<"-=-=-=-==-"<<"join";
        if(m_thread){
            if(pthread_join(m_thread, nullptr)){
                ERROR_LOG(g_logger)<< "thread :" << m_name << " pthread_join error ";
                throw std::logic_error("pthread_join error");
            }
            m_thread = 0;
        }
    }

    Thread::ptr Thread::GetThis() {
        return g_cur_thread;
    }

    const std::string &Thread::GetName() {
        return g_cur_thread_name;
    }

    pid_t Thread::GetPId() {
        if(g_cur_thread)    return g_cur_thread->getMId();
        else return GetThreadId();
    }

    void *Thread::run(void * arg) {
        Thread::ptr* td = (Thread::ptr*)arg;

        g_cur_thread = *td;
        g_cur_thread_name = g_cur_thread->m_name;
        pthread_setname_np(pthread_self(), g_cur_thread_name.substr(0, 15).c_str());
        g_cur_thread->m_id = GetThreadId();
        ERROR_LOG(g_logger)<<GetThreadId()<<"===============";
        CallBack cb;
        cb.swap(g_cur_thread->m_cb);

        g_cur_thread->m_sem.notify();
        cb();
        WARN_LOG(g_logger)<<"run finsih";
        return 0;
    }

    void Thread::start() {
        // 要注意传入的this的生命周期，如果不确定，就用智能指针
        Thread::ptr self = shared_from_this();
        if(pthread_create(&m_thread, nullptr,run,&self)){
            ERROR_LOG(g_logger)<<"pthread : "<<m_name<<" create fail";
            throw std::logic_error("pthread_create error");
        }

        // 这里用信号量的原因，就是为了保证在pthread 的run 初始化好之前，保证Thread仍然存在
        m_sem.wait();

        INFO_LOG(g_logger)<<" create "<<m_thread;
    }

    void  Thread::SetName(const std::string &name) {
        if(g_cur_thread)    g_cur_thread->m_name = name;
        g_cur_thread_name = name;
        pthread_setname_np(pthread_self(), g_cur_thread_name.substr(0, 15).c_str());
        return ;
    }

}