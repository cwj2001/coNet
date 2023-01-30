//
// Created by 抑~风 on 2023/1/30.
//
#include "mutex.h"
namespace CWJ_CO_NET{

    Semaphore::Semaphore(size_t size){
        if(sem_init(&m_sem,0,size)){
            throw std::logic_error("sem_init error");
        }
    }

    Semaphore::~Semaphore() {
        sem_destroy(&m_sem);
    }

    void Semaphore::wait() {
        if(sem_wait(&m_sem)){
            throw std::logic_error("sem_wait error");
        }
    }

    void  Semaphore::notify() {
        if(sem_post(&m_sem)){
            throw std::logic_error("sem_post error");
        }
    }

}

