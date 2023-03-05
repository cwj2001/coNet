//
// Created by 抑~风 on 2023/2/4.
//

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>


#include "log.h"
#include "fdmanager.h"


namespace CWJ_CO_NET{

    static auto g_logger = GET_LOGGER("system");

    FdCtx::FdCtx(int mFd) : m_fd(mFd) {
        init();
    }

    void FdCtx::init() {

        // 注意： 因为两个都是无符号整数，所以-1会被转为很大的数字，无限接近于阻塞
        m_recv_timeout = -1;
        m_send_timeout = -1;
        m_close = false;

        struct stat statbuf;
        memset(&statbuf,0,sizeof(statbuf));
        if(fstat(m_fd,&statbuf) == -1){
            ERROR_LOG(g_logger) << "fstat " <<m_fd<< " error,errno="<<errno<<" strerror="<<strerror(errno);
        }else{
            m_is_socket = S_ISSOCK(statbuf.st_mode);
        }

        return ;

    }

    uint64_t FdCtx::getTimeout(int type) const {
        if(type ==  SO_RCVTIMEO){
            return m_recv_timeout;
        }else{
            return m_send_timeout;
        }
        return 0;
    }

    void FdCtx::setTimeout(int type, uint64_t timeout) {
        if(type == SO_RCVTIMEO){
            m_recv_timeout = timeout;
        }else{
            m_send_timeout = timeout;
        }
    }

    FdCtx::ptr FdManager::get(int fd, bool auto_create) {
        MutexType::RLock lock(m_mutex);
        if(!m_fds.count(fd)){
            if(auto_create){
                lock.unlock();
                MutexType::WLock  lock2(m_mutex);
                if(!m_fds.count(fd))m_fds[fd] = std::make_shared<FdCtx>(fd);
            }else{
                return nullptr;
            }
        }
        lock.lock();
        return m_fds[fd];
    }

    bool FdManager::del(int fd) {

        MutexType::RLock lock(m_mutex);
        if(!m_fds.count(fd))    return false;
        lock.unlock();

        MutexType::WLock  lock1(m_mutex);

        m_fds.erase(fd);

        return true;
    }

    std::ostream &operator<<(std::ostream &os, const FdCtx &ctx) {
        os << "m_is_socket: " << ctx.m_is_socket << " m_is_nonblock: " << ctx.m_is_nonblock
           << " m_fd: " << ctx.m_fd << " m_recv_timeout: "<< ctx.m_recv_timeout
           << " m_send_timeout: " << ctx.m_send_timeout;
        return os;
    }

    bool FdCtx::isMIsNonblock() const {
        return m_is_nonblock;
    }

    void FdCtx::setMIsNonblock(bool mIsNonblock) {
        m_is_nonblock = mIsNonblock;
    }

    bool FdCtx::isMIsSocket() const {
        return m_is_socket;
    }

    bool FdCtx::isMClose() const {
        return m_close;
    }

    void FdCtx::setMClose(bool mClose) {
        m_close = mClose;
    }
}
