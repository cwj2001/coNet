//
// Created by 抑~风 on 2023/2/4.
//

#include <dlfcn.h>
#include <unistd.h>
#include <stdarg.h>

#include "hook.h"
#include "config.h"
#include "log.h"
#include "iomanager.h"
#include "macro.h"
#include "fdmanager.h"
#include "mutex.h"

namespace CWJ_CO_NET {


    static thread_local bool t_is_hook = false;

    static auto g_tcp_connect_timeout = GET_CONFIG_MGR()->lookup<int>("tcp.connect.timeout", 30000,
                                                                      "tcp connect timeout");

    static auto g_logger = GET_LOGGER("system");

#define HOOK_FUN(XX) \
    XX(sleep) \
    XX(usleep) \
    XX(nanosleep) \
    XX(socket) \
    XX(connect) \
    XX(accept) \
    XX(read) \
    XX(readv) \
    XX(recv) \
    XX(recvfrom) \
    XX(recvmsg) \
    XX(write) \
    XX(writev) \
    XX(send) \
    XX(sendto) \
    XX(sendmsg) \
    XX(close) \
    XX(fcntl) \
    XX(ioctl) \
    XX(getsockopt) \
    XX(setsockopt)


    void hook_init() {
        static bool is_inited = false;
        if (is_inited) {
            return;
        }
#define XX(name) name ## _f = (name ## _fun)dlsym(RTLD_NEXT, #name);
        HOOK_FUN(XX);
#undef XX
    }

    static uint64_t s_connect_timeout = -1;

    struct _HookIniter {
        _HookIniter() {
            hook_init();
            s_connect_timeout = g_tcp_connect_timeout->getMVal();

            g_tcp_connect_timeout->addCallBack(1, [](const int &old_value, const int &new_value) {
                INFO_LOG(g_logger) << "tcp connect timeout changed from "
                                   << old_value << " to " << new_value;
                s_connect_timeout = new_value;
            });
        }
    };

    static _HookIniter s_hook_initer;


    bool IsHookEnable() {
        return t_is_hook;
    }

    void SetHookEnable(bool flag) {
        t_is_hook = flag;
    }


    template<typename Func, typename ...Args>
    int do_io(int sockfd, Func func, std::string name, CWJ_CO_NET::IOManager::EventType event_type,
              CWJ_CO_NET::FdCtx::TimeoutType timeout_type, Args &&...args) {

        if (!CWJ_CO_NET::IsHookEnable()) {
            return func(sockfd, std::forward<Args>(args)...);
        }

//        auto fd_ctx = CWJ_CO_NET::FdMgr::GetInstance()->get(sockfd, false);

        auto fd_ctx = CWJ_CO_NET::FdMgr::GetInstance()->get(sockfd, true);

        if(fd_ctx->isMClose()){
            return func(sockfd, std::forward<Args>(args)...);
        }

//        if(!fd_ctx->isMIsSocket()){
//
//        }

        if (!fd_ctx->isMIsNonblock() ) {
            if(SetNonblock(sockfd) == -1){
                return func(sockfd, std::forward<Args>(args)...);
            }else{
                fd_ctx->setMIsNonblock(true);
            }
        }

//        if(name == "recv"){
//            CWJ_ASSERT(false);
//        }


        auto iomanager = CWJ_CO_NET::IOManager::GetThis();
        auto co = CWJ_CO_NET::Coroutine::GetThis();


        uint64_t ms = fd_ctx->getTimeout(timeout_type);

        std::shared_ptr<CWJ_CO_NET::ConditionInfo> con_info(new CWJ_CO_NET::ConditionInfo);
        std::weak_ptr<CWJ_CO_NET::ConditionInfo> con(con_info);

        CWJ_CO_NET::Timer::ptr timer = nullptr;
        if (ms != (uint64_t) -1) {
            INFO_LOG(g_logger) << "ms :" << ms;
            iomanager->addConditionTimer(ms, [con, iomanager, sockfd, event_type]() {
                auto ptr = con.lock();
                if (!ptr || ptr->isMIsCancel()) return;
                ptr->setMIsCancel(true);
                iomanager->cancelEvent(sockfd, event_type);
            }, con, false);
        }
        do {


            int rt = func(sockfd, std::forward<Args>(args)...);
            while (rt == -1 && errno == EINTR) {
                rt = func(sockfd, std::forward<Args>(args)...);
            }
            if (rt >= 0) {
                if (timer) timer->cancel();
                return rt;
            } else if (rt == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {



                iomanager->addEvent(sockfd, event_type);

                if (name == "recv") {
//                    CWJ_ASSERT(false);
                    INFO_LOG(g_logger) << "=========== recv ============";
                }

                CWJ_CO_NET::Coroutine::YieldToHold();

//                if (name == "recv") {
//                    CWJ_ASSERT(false);
//                }

                {
                    if (!con_info || con_info->isMIsCancel() == true) {
                        WARN_LOG(g_logger) << "accept timeout";
                        return -1;
                    }
                }

                continue;
            } else {
                ERROR_LOG(g_logger) << name << " error ,errno=" << errno << " strerror= " << strerror(errno);
                return -1;
            }
        } while (true);

    }


}


extern "C" {
#define XX(name) name ## _fun name ## _f = nullptr;
HOOK_FUN(XX) ;
#undef XX


static auto g_logger = GET_LOGGER("system");

int usleep(useconds_t usec) {
    if (!CWJ_CO_NET::IsHookEnable()) {
        return usleep_f(usec);
    }

    auto co = CWJ_CO_NET::Coroutine::GetThis();
    auto iomanager = CWJ_CO_NET::IOManager::GetThis();

    CWJ_ASSERT(co);
    CWJ_ASSERT(iomanager)

    iomanager->addTimer(usec / 1000, [co, iomanager]() {
        INFO_LOG(GET_ROOT_LOGGER()) << "eee sleep";
        iomanager->schedule(co, -1);
    });
    CWJ_CO_NET::Coroutine::YieldToHold();
    return 0;
}

unsigned int sleep(unsigned int seconds) {
    return sleep_f(seconds);
    return usleep(seconds * 1000 * 1000);
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
    if (CWJ_CO_NET::IsHookEnable()) {
        return nanosleep_f(req, rem);
    }
    auto co = CWJ_CO_NET::Coroutine::GetThis();
    auto iom = CWJ_CO_NET::IOManager::GetThis();
    uint64_t ms = req->tv_sec * 1000 + req->tv_nsec / 1000 / 1000;
    iom->addTimer(ms, [co, iom]() { iom->schedule(co, -1); }, false);
    CWJ_CO_NET::Coroutine::YieldToHold();
    return 0;
}

int socket(int domain, int type, int protocol) {

    int sock = socket_f(domain, type, protocol);

    if (CWJ_CO_NET::IsHookEnable()) {
        auto fd_ctx = CWJ_CO_NET::FdMgr::GetInstance()->get(sock, true);
        int flags = fcntl(sock, F_GETFL, 0);
        flags |= O_NONBLOCK;

        if (fcntl(sock, F_SETFL, flags)) {
            CWJ_ASSERT(false);
        }
        fd_ctx->setMIsNonblock(true);
        fd_ctx->setMClose(false);
    }

    return sock;

}

int connect_with_timeout(int sockfd, const struct sockaddr *addr,
                         socklen_t addrlen, uint64_t ms) {


    if (!CWJ_CO_NET::IsHookEnable()) {
        return connect_f(sockfd, addr, addrlen);
    }

    auto fd_ctx = CWJ_CO_NET::FdMgr::GetInstance()->get(sockfd, false);

    if (!fd_ctx || !fd_ctx->isMIsNonblock() || !fd_ctx->isMIsSocket() || fd_ctx->isMClose()) {
        return connect_f(sockfd, addr, addrlen);
    }

    int rt = connect_f(sockfd, addr, addrlen);

    if (rt == 0) return 0;
    else if ((rt == -1 && errno != EAGAIN && errno != EINPROGRESS)) {
        CWJ_ASSERT(errno != EINPROGRESS);
        // 出现错误
        ERROR_LOG(g_logger) << "connect error ,errno=" << errno << " strerror= " << strerror(errno);
        return rt;
    }

    auto iomanager = CWJ_CO_NET::IOManager::GetThis();
    auto co = CWJ_CO_NET::Coroutine::GetThis();

    using CWJ_CO_NET::ConditionInfo;

    std::shared_ptr<ConditionInfo> con_info(new ConditionInfo);
    con_info->setMIsCancel(false);
    std::weak_ptr<ConditionInfo> con(con_info);
    CWJ_CO_NET::Timer::ptr timer= nullptr;
    if(ms != uint64_t(-1)) {
        timer = iomanager->addConditionTimer(ms, [con, iomanager, sockfd]() {
            auto ptr = con.lock();
            if (!ptr || ptr->isMIsCancel()) {
                return;
            }
            ptr->setMIsCancel(true);
            CWJ_ASSERT(false);
            iomanager->cancelEvent(sockfd, CWJ_CO_NET::IOManager::WRITE);
        }, con, false);
    }
    rt = iomanager->addEvent(sockfd, CWJ_CO_NET::IOManager::WRITE);
    if (rt) {
        CWJ_CO_NET::Coroutine::YieldToHold();
//        INFO_LOG(g_logger) << " connect after yield";
        if (timer)timer->cancel();
        if (con_info || !con_info->isMIsCancel()) return 0;
        else {
            CWJ_ASSERT(false);
            return -1;
        }
        CWJ_ASSERT(false);

    } else {
        // 如果加不进去，那么一定出现错误，所以不需要定时器了
        if (timer)timer->cancel();
        ERROR_LOG(g_logger) << "addEvent error";
    }

    int error = 0;
    socklen_t len = sizeof(error);

    rt = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);

    if (!rt && !error) {
        return 0;
    } else {
        errno = error;
        CWJ_ASSERT(false);
        return -1;
    }


}

int connect(int sockfd, const struct sockaddr *addr,
            socklen_t addrlen) {
    return connect_with_timeout(sockfd, addr, addrlen, CWJ_CO_NET::g_tcp_connect_timeout->getMVal());
}


int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    int rt = do_io(sockfd, accept_f, "accept", CWJ_CO_NET::IOManager::READ, CWJ_CO_NET::FdCtx::RCVTIMEO, addr, addrlen);

    if (rt >= 0) {
        CWJ_CO_NET::FdMgr::GetInstance()->get(rt, true);
    }

    return rt;

}


ssize_t read(int fd, void *buf, size_t count) {
    return do_io(fd, read_f, "read", CWJ_CO_NET::IOManager::READ, CWJ_CO_NET::FdCtx::RCVTIMEO, buf, count);
}

ssize_t readv(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, readv_f, "readv", CWJ_CO_NET::IOManager::READ, CWJ_CO_NET::FdCtx::RCVTIMEO, iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    return do_io(sockfd, recv_f, "recv", CWJ_CO_NET::IOManager::READ, CWJ_CO_NET::FdCtx::RCVTIMEO, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
    return do_io(sockfd, recvfrom_f, "recvfrom", CWJ_CO_NET::IOManager::READ, CWJ_CO_NET::FdCtx::RCVTIMEO, buf, len,
                 flags,
                 src_addr, addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
    return do_io(sockfd, recvmsg_f, "recvmsg", CWJ_CO_NET::IOManager::READ, CWJ_CO_NET::FdCtx::RCVTIMEO, msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return do_io(fd, write_f, "write", CWJ_CO_NET::IOManager::WRITE, CWJ_CO_NET::FdCtx::SNDTIMEO, buf, count);
}

ssize_t writev(int fd, const struct iovec *iov, int iovcnt) {
    return do_io(fd, writev_f, "writev", CWJ_CO_NET::IOManager::WRITE, CWJ_CO_NET::FdCtx::SNDTIMEO, iov, iovcnt);
}

ssize_t send(int s, const void *msg, size_t len, int flags) {
    return do_io(s, send_f, "send", CWJ_CO_NET::IOManager::WRITE, CWJ_CO_NET::FdCtx::SNDTIMEO, msg, len, flags);
}

ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) {
    return do_io(s, sendto_f, "sendto", CWJ_CO_NET::IOManager::WRITE, CWJ_CO_NET::FdCtx::SNDTIMEO, msg, len, flags, to,
                 tolen);
}

ssize_t sendmsg(int s, const struct msghdr *msg, int flags) {
    return do_io(s, sendmsg_f, "sendmsg", CWJ_CO_NET::IOManager::WRITE, CWJ_CO_NET::FdCtx::SNDTIMEO, msg, flags);
}


int close(int fd) {
    if (!CWJ_CO_NET::IsHookEnable()) {
        return close_f(fd);
    }

    auto ctx = CWJ_CO_NET::FdMgr::GetInstance()->get(fd, false);
    if (ctx) {
        auto iom = CWJ_CO_NET::IOManager::GetThis();
        if (iom) {
            iom->cancelAll(fd);
        }
        CWJ_CO_NET::FdMgr::GetInstance()->del(fd);
    }
    return close_f(fd);
}


int fcntl(int fd, int cmd, ... /* arg */ ) {
    va_list va;
    va_start(va, cmd);
    switch (cmd) {
        case F_SETFL: {
            int arg = va_arg(va, int);
            va_end(va);
            CWJ_CO_NET::FdCtx::ptr ctx = CWJ_CO_NET::FdMgr::GetInstance()->get(fd);
            if (!ctx || ctx->isMClose() || !ctx->isMIsSocket()) {
                return fcntl_f(fd, cmd, arg);
            }
            ctx->setMIsNonblock(arg & O_NONBLOCK);
            return fcntl_f(fd, cmd, arg);
        }
            break;
        case F_GETFL: {
            va_end(va);
            int arg = fcntl_f(fd, cmd);
            CWJ_CO_NET::FdCtx::ptr ctx = CWJ_CO_NET::FdMgr::GetInstance()->get(fd);
            if (!ctx || ctx->isMClose() || !ctx->isMIsSocket()) {
                return arg;
            }
            if (ctx->isMIsNonblock()) {
                return arg | O_NONBLOCK;
            } else {
                return arg & ~O_NONBLOCK;
            }
        }
            break;
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
#ifdef F_SETPIPE_SZ
        case F_SETPIPE_SZ:
#endif
        {
            int arg = va_arg(va, int);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        }
            break;
        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
#ifdef F_GETPIPE_SZ
        case F_GETPIPE_SZ:
#endif
        {
            va_end(va);
            return fcntl_f(fd, cmd);
        }
            break;
        case F_SETLK:
        case F_SETLKW:
        case F_GETLK: {
            struct flock *arg = va_arg(va, struct flock*);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        }
            break;
        case F_GETOWN_EX:
        case F_SETOWN_EX: {
            struct f_owner_exlock *arg = va_arg(va, struct f_owner_exlock*);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        }
            break;
        default:
            va_end(va);
            return fcntl_f(fd, cmd);
    }
}

int ioctl(int d, unsigned long int request, ...) {
    va_list va;
    va_start(va, request);
    void *arg = va_arg(va, void*);
    va_end(va);

    if (FIONBIO == request) {
        bool user_nonblock = !!*(int *) arg;
        CWJ_CO_NET::FdCtx::ptr ctx = CWJ_CO_NET::FdMgr::GetInstance()->get(d);
        if (!ctx || ctx->isMClose() || !ctx->isMIsSocket()) {
            return ioctl_f(d, request, arg);
        }
        ctx->setMIsNonblock(user_nonblock);
    }
    return ioctl_f(d, request, arg);
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {
    return getsockopt_f(sockfd, level, optname, optval, optlen);
}

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    if (!CWJ_CO_NET::IsHookEnable()) {
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }
    if (level == SOL_SOCKET) {
        if (optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
            CWJ_CO_NET::FdCtx::ptr ctx = CWJ_CO_NET::FdMgr::GetInstance()->get(sockfd);
            if (ctx) {
                const timeval *v = (const timeval *) optval;
                ctx->setTimeout(optname, v->tv_sec * 1000 + v->tv_usec / 1000);
            }
        }
    }
    return setsockopt_f(sockfd, level, optname, optval, optlen);
}


}
