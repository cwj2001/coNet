//
// Created by 抑~风 on 2023/1/30.
//


#include "thread.h"
#include "log.h"
#include "config.h"
#include "util.h"

namespace CWJ_CO_NET{

    static auto g_default_main_thread_name
                                = GET_CONFIG_MGR()->
                                        lookup<std::string>("cwj_co_net.thread.root_name","main","thread root name");

    static auto g_logger = GET_LOGGER("system");
    static thread_local Thread::ptr g_cur_thread  = nullptr;
    static thread_local std::string g_cur_thread_name = "unnamed";//g_default_main_thread_name->getMVal() ;

    Thread::Thread(const std::string &mName, const Thread::CallBack &cb)
                                                : m_name(mName)
                                                , m_cb(cb)
                                                , m_sem(0){
        INFO_LOG(g_logger)<<" Thread::Thread";
    }

    Thread::~Thread() {
        INFO_LOG(g_logger)<<"Thread::~Thread()";
        if(m_thread) {
            pthread_detach(m_thread);
            m_thread = 0;
        }
    }

    pid_t Thread::getMId() const {
        return m_id;
    }

    const std::string &Thread::getMName() const {
        return m_name;
    }

    void Thread::join() {
        if(m_thread){
            if(pthread_join(m_thread, nullptr)){
                ERROR_LOG(g_logger)<< "thread :" << m_name << " pthread_join error ";
                throw std::logic_error("pthread_join error");
            }
            m_thread = 0;
        }
    }

    Thread::ptr Thread::GetThis(const std::string &name) {

        // 这个判断只有在主线程中才能会被进入，如果是采用pthread_create函数开辟的线程，
        // 那么就会在启动线程的时候设置，且因为该变量是线程私有，所以不需要担心线程安全问题
        if(!g_cur_thread){
            g_cur_thread = std::shared_ptr<Thread>(new Thread(name));
        }
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
        CallBack cb;
        cb.swap(g_cur_thread->m_cb);

        g_cur_thread->m_sem.notify();
        cb();

        g_cur_thread_name = "";
        g_cur_thread.reset();

        return 0;
    }

    void Thread::start() {
        // 要注意传入的this的生命周期，如果不确定，就用智能指针
        Thread::ptr self = shared_from_this();
        pthread_t t;
        if(pthread_create(&t, nullptr,run,&self)){
            ERROR_LOG(g_logger)<<"pthread : "<<m_name<<" create fail";
            throw std::logic_error("pthread_create error");
        }

        m_thread = t;

        // 这里用信号量的原因，就是为了保证在pthread 的run 初始化好之前，保证Thread仍然存在
        m_sem.wait();

    }

    void  Thread::SetName(const std::string &name) {
        if(g_cur_thread)    g_cur_thread->m_name = name;
        g_cur_thread_name = name;
        pthread_setname_np(pthread_self(), g_cur_thread_name.substr(0, 15).c_str());
        return ;
    }

    void Thread::SetThread(Thread::ptr t) {
        g_cur_thread = t;
    }

    Thread::Thread(const std::string &name) : m_id(GetThreadId())
            ,m_name(name)
            , m_sem(0) {
    }

}