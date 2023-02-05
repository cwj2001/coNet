//
// Created by 抑~风 on 2023/2/4.
//

#ifndef CWJ_CO_NET_FDMANAGER_H
#define CWJ_CO_NET_FDMANAGER_H

#include <inttypes.h>
#include <unordered_map>
#include <memory>

#include "mutex.h"
#include "singleton.h"

namespace CWJ_CO_NET{

    class FdCtx{
    public:

        enum TimeoutType{
            RCVTIMEO = SO_RCVTIMEO,
            SNDTIMEO = SO_SNDTIMEO
        };

        using ptr = std::shared_ptr<FdCtx>;

        FdCtx(int mFd);

    private:
        void init();

    public:

        bool isMIsNonblock() const;

        void setMIsNonblock(bool mIsNonblock);

        uint64_t getTimeout(int typr) const;

        void setTimeout(int type,uint64_t timeout);

        friend std::ostream &operator<<(std::ostream &os, const FdCtx &ctx);

        bool isMIsSocket() const;

        bool isMClose() const;

        void setMClose(bool mClose);


    private:

        bool m_is_socket{0};
        bool m_is_nonblock{0};
        int m_fd{0};
        bool m_close{false};
        uint64_t m_recv_timeout{(uint64_t)-1};
        uint64_t m_send_timeout{(uint64_t)-1};
    };

    class FdManager{
    public:
        using ptr = std::shared_ptr<FdManager>;
        using MutexType = RWMutex;

        FdCtx::ptr get(int fd,bool auto_create = true);

        bool del(int fd);

    private:
        std::unordered_map<int,FdCtx::ptr> m_fds;
        MutexType m_mutex;
    };

    using FdMgr = Singleton<FdManager>;

}

#endif //CWJ_CO_NET_FDMANAGER_H
