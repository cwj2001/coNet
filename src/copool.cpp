//
// Created by 抑~风 on 2023-03-04.
//

#include <cmath>
#include <algorithm>
#include "config.h"
#include "copool.h"

namespace CWJ_CO_NET{

    static thread_local std::queue<Coroutine::ptr> t_co_que;
    static const int THREAD_MAX_CO = 16;
    static const int CENTER_MAX_CO = 32;
    static auto g_thread_max_co = GET_CONFIG_MGR()->lookup<int>("copool.thread_max_co",THREAD_MAX_CO,"thread_max_co");
    static auto g_center_max_co = GET_CONFIG_MGR()->lookup<int>("copool.center_max_co",CENTER_MAX_CO,"center_max_co");

    static int init_t = [](){

        if(g_thread_max_co->getMVal() <= 0) g_thread_max_co->setMVal(THREAD_MAX_CO);
        if(g_center_max_co->getMVal() <= 0) g_center_max_co->setMVal(CENTER_MAX_CO);

        int key = 0x1234;

        g_center_max_co->addCallBack(key,[](const int& oldVal,const int& newVal){
            if(oldVal == newVal)    return ;
            auto pool = CoPoolMgr::GetInstance();
            pool->setMCenterMaxCo(newVal);
        });

        g_thread_max_co->addCallBack(key,[](const int& oldVal,const int& newVal){
            if(oldVal == newVal)    return ;
            auto pool = CoPoolMgr::GetInstance();
            pool->setMThreadMaxCo(newVal);
        });

        return 0;
    }();

    Coroutine::ptr CoroutinePool::allocCo(CoroutinePool::CallBack cb,size_t stack_size,bool use_scheduler) {


        // 私有空间有
        if(t_co_que.size()){
            auto c = t_co_que.front();
            t_co_que.pop();
            c->reset(cb,stack_size,use_scheduler);
            return c;
        }

        // 共享空间

        while(m_co_center_size > 0){
            MutexType::Lock lock(m_mutex);
            if(m_co_center_size <= 0)   continue;
            auto c = m_co_center.front();
            m_co_center.pop();
            c->reset(cb,stack_size,use_scheduler);
            return c;
        }


        // 创建新协程

        return std::make_shared<Coroutine>(cb,stack_size,use_scheduler);

    }

    void CoroutinePool::deallocCo(Coroutine::ptr co) {

        // 处理私有空间

        if(m_thread_max_co > t_co_que.size()){
            t_co_que.push(co);
            return ;
        }

        while(m_center_max_co>m_co_center.size()){
            MutexType::Lock lock(m_mutex);
            if(m_thread_max_co<=m_co_center.size()) continue;
            m_co_center.push(co);
            return ;
        }

        co.reset();

        return ;

    }

    CoroutinePool::CoroutinePool(){}

    const std::atomic<uint64_t> &CoroutinePool::getMThreadMaxCo() const {
        return m_thread_max_co;
    }

    void CoroutinePool::setMThreadMaxCo(uint64_t mThreadMaxCo) {
        if(mThreadMaxCo > 0)   m_thread_max_co = mThreadMaxCo;
    }

    const std::atomic<uint64_t> &CoroutinePool::getMCenterMaxCo() const {
        return m_center_max_co;
    }

    void CoroutinePool::setMCenterMaxCo(uint64_t mCenterMaxCo) {
        if(mCenterMaxCo > 0)  m_center_max_co = mCenterMaxCo;
    }
}