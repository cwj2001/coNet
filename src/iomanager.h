//
// Created by 抑~风 on 2023/2/3.
//

#ifndef CWJ_CO_NET_IOMANAGER_H
#define CWJ_CO_NET_IOMANAGER_H

#include <memory>
#include <functional>
#include <sys/epoll.h>
#include <atomic>
#include <map>


#include "scheduler.h"
#include "mutex.h"
#include "coroutine.h"


namespace CWJ_CO_NET {

    class IOManager : public Scheduler {
    public:
        using ptr = std::shared_ptr<IOManager>;
        using MutexType = RWMutex;

        enum EventType {
            NONE = 0,
            READ = EPOLLIN,
            WRITE = EPOLLOUT,
        };

        struct FdContext {
            using MutexType = Mutex;
            struct EventContext {
                CallBack m_cb;
                Coroutine::ptr m_co;
                Scheduler::ptr m_scheduler;

                void reset();
            };

            FdContext();

            EventContext &getContextFromType(EventType type);

            // 便于复用
            void reset();

            // 触发事件
            bool triggerEvent(EventType type);

            EventContext m_read_ev;
            EventContext m_write_ev;
            int m_fd;
            EventType m_types;
            MutexType m_mutex;
        };

        IOManager(size_t size, bool useCurThread, const std::string &name);

        ~IOManager();

        void wake() override;

        void idle() override;

        bool addEvent(int fd, EventType event, CallBack cb = nullptr);

        bool delEvent(int fd, EventType event);

        bool cancelEvent(int fd, EventType event);

        bool cancelAll(int fd);

    public:

        static IOManager::ptr GetThis();


    private:
        int m_epoll_fd = -1;
        int m_wakeup_fd = -1;
        MutexType m_mutex;
        std::atomic<size_t> m_pending_event_count{0};
        std::map<int, FdContext *> m_fd_contexts;
    };

}


#endif //CWJ_CO_NET_IOMANAGER_H
