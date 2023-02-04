//
// Created by 抑~风 on 2023/1/30.
//

#ifndef CWJ_CO_NET_MUTEX_H
#define CWJ_CO_NET_MUTEX_H

#include <semaphore.h>
#include <stdexcept>
#include <pthread.h>
#include <atomic>

#include "noncopyable.h"
namespace CWJ_CO_NET{

    class Semaphore{
    public:
        Semaphore(size_t size);
        ~Semaphore();
        void wait();
        void notify();
    private:
        sem_t m_sem;
    };

    template<typename T>
    class ScopeLock : NonCopyAble{
    public:
        ScopeLock(T& t):m_mutex(t){
            m_mutex.lock();
            m_isLock = true;
        }

        void unlock(){
            if(m_isLock){
                m_mutex.unlock();
            }
        }

        void lock(){
            if(!m_isLock){
                m_mutex.lock();
            }
        }

        ~ScopeLock(){
            if(m_isLock){
                m_mutex.unlock();
                m_isLock = false;
            }
        }
    private:
        T& m_mutex;
        bool m_isLock;
    };


    template<typename T>
    class RScopeLock : NonCopyAble{
    public:
        RScopeLock(T& t):m_mutex(t),m_isLock(false)  {
            m_mutex.rdlock();
            m_isLock=true;
        }

        ~RScopeLock() {
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
                m_mutex.unlock();
                m_isLock = false;
            }
        }

    private:
        T& m_mutex;
        bool m_isLock;
    };
    template<typename T>
    class WScopeLock : NonCopyAble{
    public:
        WScopeLock(T& t):m_mutex(t),m_isLock(false)  {
            m_mutex.wrlock();
            m_isLock=true;
        }

        ~WScopeLock() {
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
                m_mutex.unlock();
                m_isLock = false;
            }
        }

    private:
        T& m_mutex;
        bool m_isLock;
    };


    class Mutex : NonCopyAble{
    public:

        using Lock = ScopeLock<Mutex>;

        Mutex(){ pthread_mutex_init(&m_mutex, nullptr); };
        ~Mutex(){pthread_mutex_destroy(&m_mutex);};
        void unlock(){pthread_mutex_unlock(&m_mutex);};
        void lock(){pthread_mutex_lock(&m_mutex);};
    private:
        pthread_mutex_t m_mutex;
    };

    class RWMutex : NonCopyAble{
    public:

        using RLock = RScopeLock<RWMutex>;
        using WLock = WScopeLock<RWMutex>;

        RWMutex(){pthread_rwlock_init(&m_rwlock, nullptr);};
        ~RWMutex(){pthread_rwlock_destroy(&m_rwlock);};
        void rdlock(){pthread_rwlock_rdlock(&m_rwlock);};
        void wrlock(){pthread_rwlock_wrlock(&m_rwlock);};
        void unlock(){pthread_rwlock_unlock(&m_rwlock);};
    private:
        pthread_rwlock_t  m_rwlock;
    };

    class SpinLock : NonCopyAble{
    public:

        using Lock = ScopeLock<SpinLock>;

        SpinLock(){pthread_spin_init(&m_spinlock,0);};
        ~SpinLock(){pthread_spin_destroy(&m_spinlock);};
        void unlock(){pthread_spin_unlock(&m_spinlock);};
        void lock(){pthread_spin_lock(&m_spinlock);};
    private:
        pthread_spinlock_t m_spinlock;
    };


    class CASLock : NonCopyAble{
    public:
        using Lock = ScopeLock<CASLock>;
        CASLock(){
            m_flag.clear();
        };

         ~CASLock() {}

         void lock(){
             while(std::atomic_flag_test_and_set_explicit(&m_flag,std::memory_order_acquire));
         }

         void unlock(){
             std::atomic_flag_clear_explicit(&m_flag,std::memory_order_acquire);
         }

    private:
        std::atomic_flag m_flag;
    };

}

#endif //CWJ_CO_NET_MUTEX_H
