//
// Created by 抑~风 on 2023/1/30.
//

#ifndef CWJ_CO_NET_THREAD_H
#define CWJ_CO_NET_THREAD_H

#include <pthread.h>
#include <string>
#include <functional>
#include <memory>
#include <stdexcept>

#include "mutex.h"
#include "noncopyable.h"


namespace CWJ_CO_NET{

class Thread : public std::enable_shared_from_this<Thread>,public NonCopyAble {
    public:
        using ptr = std::shared_ptr<Thread>;
        using CallBack = std::function<void()>;

        Thread(const std::string &mName, const CallBack &cb);
        ~Thread();

        void start();
        void join();

        pid_t getMId() const;

        const std::string &getMName() const;

    public:

        static Thread::ptr GetThis();
        static const std::string& GetName();
        static pid_t GetPId();
        static void SetName(const std::string& name);
    private:

        static void* run(void *);

    private:
        pid_t m_id = 0;
        std::atomic<pthread_t> m_thread{0};
        std::string m_name;
        CallBack m_cb;
        Semaphore m_sem;
    };

}

#endif //CWJ_CO_NET_THREAD_H
