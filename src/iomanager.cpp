//
// Created by 抑~风 on 2023/2/3.
//
#include "iomanager.h"
#include "macro.h"
#include <sys/eventfd.h>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>


#include "log.h"

namespace CWJ_CO_NET {

    static auto g_logger = GET_LOGGER("system");


    void IOManager::wake() {
        if (m_idle_thread_count <= 0) return;
        uint64_t u = 1;
        if (write(m_wakeup_fd, &u, sizeof(uint64_t)) == -1) {
            CWJ_ASSERT(false);
            ERROR_LOG(g_logger) << "wake write fail ,errno=" << errno << " strerror= " << strerror(errno);
        }
    }

    void IOManager::idle() {


        static uint64_t MAX_EVENTS = 256;
        static std::shared_ptr<epoll_event> events(new epoll_event[MAX_EVENTS], [](auto &a) { delete[]a; });

        do {
            int len = 0;
            do {
                static uint64_t  MAX_TIMEOUT = 3000ul;
                uint64_t ms = getNextTimer();
                ms = ms > MAX_TIMEOUT ? MAX_TIMEOUT : ms;
                INFO_LOG(g_logger)<<"ms:"<<ms;
                len = epoll_wait(m_epoll_fd, events.get(), MAX_EVENTS, ms);
                if (len >0 || (len == 0 && EFAULT)) break;
                else{
                    ERROR_LOG(g_logger) << "epoll_wait error ,errno="<<errno<<" strerror="<<strerror(errno);
                }
            } while (true);


            std::vector<TimerManager::CallBack>list;
            listExpiredCb(list);
            for(auto a : list){
                schedule(a,-1);
            }


            auto evs = events.get();
            for (int i = 0; i < len; i++) {
                if (evs[i].data.fd == m_wakeup_fd) {
                    uint64_t u;
                    read(m_wakeup_fd, &u, sizeof(u));
                    continue;
                }
                epoll_event &event = evs[i];
                auto fd_context = (FdContext *) evs[i].data.ptr;

                int real_event = ((EPOLLIN | EPOLLOUT) & event.events) & fd_context->m_types;

                if (real_event == NONE) continue;



                fd_context->m_types = (EventType) (fd_context->m_types & ~(real_event));
                event.events = fd_context->m_types | EPOLLET;
                int op = fd_context->m_types == NONE ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;
                int rt = epoll_ctl(m_epoll_fd, op, fd_context->m_fd, &evs[i]);

                // 要是操作不超过，那么就不执行任务；
                if (rt) {
                    ERROR_LOG(g_logger) << "epoll_ctl(" << m_epoll_fd << ", "
                                        << op << ", " << fd_context->m_fd << ", " << (EPOLL_EVENTS) evs[i].events
                                        << "):"
                                        << rt << " (" << errno << ") (" << strerror(errno) << ")";
                    continue;
                }

                if (real_event & EPOLLIN) {
                    fd_context->triggerEvent(READ);
                    --m_pending_event_count;
                }

                if (real_event & EPOLLOUT) {
                    fd_context->triggerEvent(WRITE);
                    --m_pending_event_count;
                }

                if (fd_context->m_types == NONE) fd_context->reset();

            }
        }while(getTaskCount()<=0);

    }

    IOManager::IOManager(size_t size, bool useCurThread, const std::string &name) : Scheduler(size, useCurThread,
                                                                                              name) {
        m_epoll_fd = epoll_create(1024);
        m_wakeup_fd = eventfd(0, 0);

        epoll_event event;
        memset(&event, 0, sizeof(event));
        event.events = EPOLLIN | EPOLLET;
        event.data.fd = m_wakeup_fd;

        if (fcntl(m_wakeup_fd, F_SETFL, O_NONBLOCK)) {
            CWJ_ASSERT(false);
        }

        if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_wakeup_fd, &event) == -1) {
            ERROR_LOG(g_logger) << "epoll add wakeup_fd error ,errno=" << errno << " str(errno)= " << strerror(errno);
            CWJ_ASSERT(false);
        }
    }

    IOManager::~IOManager() {
        close(m_epoll_fd);
        close(m_wakeup_fd);
        for (auto &p : m_fd_contexts) {
            delete p.second;
        }
    }

    bool IOManager::addEvent(int fd, IOManager::EventType event_type, Scheduler::CallBack cb) {

        MutexType::RLock lock(m_mutex);
        if (!m_fd_contexts.count(fd)) {
            m_fd_contexts[fd] = new FdContext;
            m_fd_contexts[fd]->reset();
        }
        auto &fd_ctx = m_fd_contexts[fd];

        lock.unlock();

        FdContext::MutexType::Lock lock2(fd_ctx->m_mutex);
        int op = fd_ctx->m_types == NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

        epoll_event event;
        memset(&event, 0, sizeof(event));
        event.events = (fd_ctx->m_types | event_type) | EPOLLET;
        event.data.ptr = m_fd_contexts[fd];


        int rt = epoll_ctl(m_epoll_fd, op, fd, &event);
        if (rt) {
            ERROR_LOG(g_logger) << "addEvent(fd=" << fd << " event_type="
                                << event_type << ") epoll_ctl error ,errno=" << errno
                                << " strerror= " << strerror(errno);
            return false;
        }


        fd_ctx->m_types = EventType(fd_ctx->m_types | event_type);
        fd_ctx->m_fd = fd;
        auto &ev_ctx = fd_ctx->getContextFromType(event_type);
        if(cb){
            ev_ctx.m_cb.swap(cb);
        }else{
            ev_ctx.m_co = Coroutine::GetThis();
        }

        ev_ctx.m_scheduler = shared_from_this();

        ++m_pending_event_count;

//        INFO_LOG(g_logger)<<" event.data.fd="<< ((FdContext*)event.data.ptr)->m_fd << "fd = "<<fd;


        return true;
    }

    bool IOManager::delEvent(int fd, IOManager::EventType event_type) {

        MutexType::RLock lock(m_mutex);
        if (!m_fd_contexts.count(fd)) {
            return false;
        }

        auto &fd_ctx = m_fd_contexts[fd];
        lock.unlock();

        FdContext::MutexType::Lock lock2(fd_ctx->m_mutex);

        if (!(event_type & fd_ctx->m_types)) return false;

        epoll_event event;
        memset(&event, 0, sizeof(event));
        event.events = (fd_ctx->m_types & ~event_type) | EPOLLET;
        int op = event.events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
        event.data.fd = fd;
        event.data.ptr = fd_ctx;
        int rt = epoll_ctl(m_epoll_fd, op, fd, &event);
        if (rt) {
            ERROR_LOG(g_logger) << "addEvent(fd=" << fd << " event_type="
                                << event_type << ") epoll_ctl error ,errno=" << errno
                                << " strerror= " << strerror(errno);
            return false;
        }

        --m_pending_event_count;
        fd_ctx->m_types = (EventType) (fd_ctx->m_types & ~event_type);
        fd_ctx->m_fd = fd;
        if (event_type == READ) {
            fd_ctx->m_read_ev.reset();
        } else if (event_type == WRITE) {
            fd_ctx->m_write_ev.reset();
        }
        return true;
    }

    bool IOManager::cancelEvent(int fd, IOManager::EventType event_type) {
        MutexType::RLock lock(m_mutex);
        if (!m_fd_contexts.count(fd)) {
            return false;
        }

        auto &fd_ctx = m_fd_contexts[fd];
        lock.unlock();

        FdContext::MutexType::Lock lock2(fd_ctx->m_mutex);

        if (!(event_type & fd_ctx->m_types)) return false;

        epoll_event event;
        memset(&event, 0, sizeof(event));
        event.events = (fd_ctx->m_types & ~event_type) | EPOLLET;
        int op = event.events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
        event.data.fd = fd;
        event.data.ptr = fd_ctx;
        int rt = epoll_ctl(m_epoll_fd, op, fd, &event);
        if (rt) {
            ERROR_LOG(g_logger) << "cancelEvent(fd=" << fd << " event_type="
                                << event_type << ") epoll_ctl error ,errno=" << errno
                                << " strerror= " << strerror(errno);
            return false;
        }

        --m_pending_event_count;
        fd_ctx->m_types = (EventType)(fd_ctx->m_types & ~event_type);
        fd_ctx->m_fd = fd;
        if (event_type == READ) {
            fd_ctx->triggerEvent(event_type);
            fd_ctx->m_read_ev.reset();
        } else if (event_type == WRITE) {
            fd_ctx->triggerEvent(event_type);
            fd_ctx->m_write_ev.reset();
        }
        return true;
    }

    bool IOManager::cancelAll(int fd) {
        MutexType::RLock lock(m_mutex);
        if (!m_fd_contexts.count(fd)) {
            return false;
        }

        auto &fd_ctx = m_fd_contexts[fd];
        lock.unlock();

        FdContext::MutexType::Lock lock2(fd_ctx->m_mutex);

        if (!(fd_ctx->m_types)) return false;

        epoll_event event;
        memset(&event, 0, sizeof(event));
        event.events = NONE;
        int op = EPOLL_CTL_DEL;
        event.data.fd = fd;
        event.data.ptr = fd_ctx;
        int rt = epoll_ctl(m_epoll_fd, op, fd, &event);
        if (rt) {
            ERROR_LOG(g_logger) << "cancelEvent(fd=" << fd << " event_type="
                                << NONE << ") epoll_ctl error ,errno=" << errno
                                << " strerror= " << strerror(errno);
            return false;
        }


        if (fd_ctx->triggerEvent(READ)) {
            --m_pending_event_count;
        }
        if (fd_ctx->triggerEvent(WRITE)) {
            --m_pending_event_count;
        }
        return true;
    }

    IOManager::ptr IOManager::GetThis() {
        return std::dynamic_pointer_cast<IOManager>(Scheduler::GetThis());
    }

    void IOManager::onTimerInsertedAtFront() {
        wake();
    }


    IOManager::FdContext::FdContext() {
        this->reset();
    }

    IOManager::FdContext::EventContext &IOManager::FdContext::getContextFromType(IOManager::EventType type) {
        switch (type) {
            case READ:
                return m_read_ev;
                break;
            case WRITE:
                return m_write_ev;
                break;
            default:
                CWJ_ASSERT(false);
        }
        throw std::invalid_argument("getContext invalid event");
    }

    void IOManager::FdContext::reset() {
        this->m_fd = -1;
        this->m_types = NONE;
        this->m_read_ev.reset();
        this->m_write_ev.reset();
    }

    bool IOManager::FdContext::triggerEvent(EventType type) {

        if (type == NONE || type & m_types) return false;

        CWJ_ASSERT(type == READ || type == WRITE);

        m_types = (EventType) (m_types & (~type));

        auto &ctx = this->getContextFromType(type);

        if (ctx.m_co) {
            CWJ_ASSERT(ctx.m_co->getMState() == CoState::State::HOLD);
            ctx.m_co->setMState(CoState::READY);
            ctx.m_scheduler->schedule(ctx.m_co, -1);
        } else if (ctx.m_cb) {
            ctx.m_scheduler->schedule(ctx.m_cb, -1);
        } else {
            CWJ_ASSERT(false);
        }
        return true;
    }

    void IOManager::FdContext::EventContext::reset() {
        this->m_co.reset();
        this->m_scheduler.reset();
        this->m_cb = nullptr;
    }
}
