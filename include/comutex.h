//
// Created by 抑~风 on 2023-03-27.
//

#ifndef CWJ_CO_NET_COMUTEX_H
#define CWJ_CO_NET_COMUTEX_H

#include <atomic>
#include <sys/eventfd.h>
#include <unistd.h>

#include "mutex.h"
namespace CWJ_CO_NET {

    class CoSemaphore{
    public:
        using MutexType = Mutex;
        CoSemaphore(int64_t size = 0) : m_num(size) {
            m_ev_fd = eventfd(0,0);
        };
        void wait(){
            // 加锁的原因是为了让修改和
            MutexType::Lock lock(m_mutex);
            m_num -- ;
            if(m_num < 0){
                // wait;
                lock.unlock();
                uint64_t buf;
                read(m_ev_fd,&buf,sizeof(uint64_t));
            }
        }
        void notify(){
            MutexType::Lock lock(m_mutex);
            m_num ++ ;
            if(m_num >= 0){
                lock.unlock();
                // notify
                uint64_t  buf = 0;
                write(m_ev_fd,&buf,sizeof(uint64_t));
            }
        }
    private:
        std::atomic_int64_t m_num{0};
        eventfd_t m_ev_fd{0};
        MutexType m_mutex;
    };

    class CoMutex{
    public:

        using Lock = ScopeLock<CoMutex>;

        CoMutex(){
            m_ev_d = eventfd(0,0);
        }

        void lock(){
            uint64_t u = 1;
            read(m_ev_d,&u,sizeof(uint64_t));
        }

        void unlock(){
            uint64_t u = 1;
            write(m_ev_d,&u,sizeof(uint64_t));
        }

    private:
        eventfd_t m_ev_d;
    };

    template<typename T>
    class CoRScopeLock : NonCopyAble{
    public:
        CoRScopeLock(T& t):m_mutex(t),m_isLock(false)  {
            m_mutex.rdlock();
            m_isLock=true;
        }

        ~CoRScopeLock() {
            if(m_isLock){
                m_mutex.unlock();
            }
        }

        void lock(){
            if(!m_isLock){
                m_mutex.rdlock();
                m_isLock = true;
            }
        }

        void unlock(){
            if(m_isLock) {
                m_mutex.unrlock();
                m_isLock = false;
            }
        }

    private:
        T& m_mutex;
        bool m_isLock;
    };
    template<typename T>
    class CoWScopeLock : NonCopyAble{
    public:
        CoWScopeLock(T& t):m_mutex(t),m_isLock(false)  {
            m_mutex.wrlock();
            m_isLock=true;
        }

        ~CoWScopeLock() {
            if(m_isLock){
                m_mutex.unlock();
            }
        }

        void lock(){
            if(!m_isLock){
                m_mutex.wrlock();
                m_isLock = true;
            }
        }

        void unlock(){
            if(m_isLock) {
                m_mutex.unwlock();
                m_isLock = false;
            }
        }

    private:
        T& m_mutex;
        bool m_isLock;
    };


    class CoRWMutex{
    public:
        using RLock = CoRScopeLock<RWMutex>;
        using WLock = CoWScopeLock<RWMutex>;

        CoRWMutex(){

        };
        ~CoRWMutex(){

        };
        void rdlock(){
            m_flag_sem.wait();
            m_r_count_sem.wait();
            if(m_r_count == 0){
                m_w_data_sem.wait();
            }
            m_r_count ++ ;
            m_flag_sem.notify();
        };
        void wrlock(){
            m_flag_sem.wait();
            m_w_data_sem.wait();
        };
        void unrlock(){
            m_r_count_sem.wait();
            m_r_count --;
            if(m_r_count == 0){
                m_w_data_sem.notify();
            }
            m_r_count_sem.notify();
        };
        void unwlock(){
            m_w_data_sem.notify();
            m_flag_sem.notify();
        }
    private:
        CoSemaphore m_flag_sem{1};
        CoSemaphore m_r_count_sem{1};
        CoSemaphore m_w_data_sem{1};
        std::atomic_int64_t m_r_count{0};
    };

}

#endif //CWJ_CO_NET_COMUTEX_H
