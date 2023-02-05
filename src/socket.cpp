//
// Created by 抑~风 on 2023/2/2.
//

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>


#include <sstream>


#include "socket.h"
#include "log.h"
#include "macro.h"
#include "fdmanager.h"

namespace CWJ_CO_NET{

    static auto g_logger = GET_LOGGER("system");

    Socket::~Socket() {

    }

    Socket::Socket(int mFamily, int mType, int mProtocol) : m_family(mFamily), m_type(mType), m_protocol(mProtocol) {}

    int Socket::getMFamily() const {
        return m_family;
    }

    int Socket::getMType() const {
        return m_type;
    }

    int Socket::getMProtocol() const {
        return m_protocol;
    }

    bool Socket::isConnect() const {
        return m_is_connect;
    }

    const Address::ptr Socket::getMLocalAddr() {

        if(m_local_addr)    return m_local_addr;

        Address::ptr res = nullptr;

        switch (m_family) {
            case Family::IPv4:
                res.reset(new IPv4Address);
                break;
            case Family::IPv6:
                res.reset(new IPv6Address);
                break;
            case Family::UNIX:
                res.reset(new UnixAddress);
                break;
            default:
                res.reset(new UnknownAddress);
        }

        auto len = res->getAddrLen();
        if(getsockname(m_sock,res->getAddr(),&len)){
            WARN_LOG(g_logger)<<" getsockname fail sock ="<<m_sock
                              << " errno=" << errno << " errstr="
                              << strerror(errno);
            res.reset(new UnknownAddress(m_family));
            return res;
        }

        if(m_family == AF_UNIX) {
            UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(res);
            addr->setMLen(len);
        }
        m_local_addr = res;

        return m_local_addr;
    }

    const Address::ptr Socket::getMRemoteAddr() {
        if(m_remote_addr)    return m_remote_addr;

        Address::ptr res = nullptr;

        switch (m_family) {
            case Family::IPv4:
                res.reset(new IPv4Address);
                break;
            case Family::IPv6:
                res.reset(new IPv6Address);
                break;
            case Family::UNIX:
                res.reset(new UnixAddress);
                break;
            default:
                res.reset(new UnknownAddress);
        }

        auto len = res->getAddrLen();
        if(getpeername(m_sock,res->getAddr(),&len)){
            WARN_LOG(g_logger)<<" getpeername fail sock ="<<m_sock
                              << " errno=" << errno << " errstr="
                              << strerror(errno);
            res.reset(new UnknownAddress(m_family));
            return res;
        }

        if(m_family == AF_UNIX) {
            UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(res);
            addr->setMLen(len);
        }
        m_remote_addr = res;

        return m_remote_addr;
    }

    bool Socket::bind(const Address::ptr addr) {

        if(!isValid()){
            if(!newSocket()) {
                return false;
            }else{
                m_family = addr->getFamily();
            }
        }

        if(m_family != addr->getFamily()){
            ERROR_LOG(g_logger) << "socket(family="<<m_family<<") bind addr(family="<<addr->getFamily()<<") fail";
            return false;
        }

        // 对于Unix的地址，其路径必须是不存在的，不然无法被bind，所以需要判断是否存在当前路径的连接，有就解除
        const UnixAddress::ptr unix_ptr = std::dynamic_pointer_cast<UnixAddress>(addr);

        if(unix_ptr){
            auto tmp = Socket::CreateUnixTCPSocket();
            if(!tmp->connect(unix_ptr)){
                ::unlink(unix_ptr->getPath().c_str());
            }
            tmp->close();
        }

        if(::bind(m_sock,addr->getAddr(),addr->getAddrLen())){
            ERROR_LOG(g_logger) << "sock bind "<<addr
                                << "fail  errrno=" << errno
                                <<  "errstr=" << strerror(errno);
            return false;
        }
        getMLocalAddr();
        return true;
    }

    bool Socket::listen(int backlog) {
        if(!isValid()){
            ERROR_LOG(g_logger) << "listen error sock=-1";
            return false;
        }
        if(::listen(m_sock,backlog)){
            ERROR_LOG(g_logger) << "listen error errno=" << errno
                                << " errstr=" << strerror(errno);
            return false;
        }

        return true;
    }

    Socket::ptr Socket::accept() {

        Socket::ptr sock(new Socket(m_family,m_type,m_protocol));

        int newsock = ::accept(m_sock, nullptr, nullptr);

        if(newsock == -1) {
            ERROR_LOG(g_logger) << "accept(" << m_sock << ") errno="
                                      << errno << " errstr=" << strerror(errno);
            return nullptr;
        }
        if(sock->init(newsock,true)){
            sock->setNonBlock();
            return sock;
        }
        return nullptr;
    }

    bool Socket::close() {
        if(!m_is_connect && m_sock == -1)   return true;
        m_is_connect = false;
        if(m_sock != -1){
            ::close(m_sock);
            m_sock = -1;
        }
        return false;
    }

    bool Socket::connect(const Address::ptr addr, uint64_t timeout_ms) {

        if(!isValid()){
            if(!newSocket()) {
                return false;
            }else{
                m_family = addr->getFamily();
            }
        }

        if(m_family != addr->getFamily()){
            ERROR_LOG(g_logger) << "socket(family="<<m_family<<") bind addr(family="<<addr->getFamily()<<") fail";
            return false;
        }

        if(timeout_ms == (uint64_t)-1) {
            if(::connect(m_sock, addr->getAddr(), addr->getAddrLen())) {
                ERROR_LOG(g_logger) << "sock=" << m_sock << " connect(" << addr->toString()
                                          << ") error errno=" << errno << " errstr=" << strerror(errno);
                close();
                return false;
            }
        } else {
            // TODO 目前没有涉及到定时器，所以超时机制还不能实现
            if(true) {
                ERROR_LOG(g_logger) << "sock=" << m_sock << " connect(" << addr->toString()
                                          << ") timeout=" << timeout_ms << " error errno="
                                          << errno << " errstr=" << strerror(errno);
                close();
                return false;
            }
        }
        m_is_connect = true;
        getMLocalAddr();
        getMRemoteAddr();
        return true;
    }

    bool Socket::reconnect(uint64_t timeout_ms) {
        if(!m_remote_addr){
            ERROR_LOG(g_logger) << "reconnect m_remoteAddress is null";
            return false;
        }
        m_local_addr.reset();
        return connect(m_remote_addr,timeout_ms);
    }

    int Socket::send(const void *buffer, size_t length, int flags) {
        if(isConnect()){
            return ::send(m_sock,buffer,length,flags);
        }
        return -1;
    }

    int Socket::send(const iovec *buffers, size_t length, int flags) {
        if(isConnect()){
            msghdr msg;
            memset(&msg,0,sizeof (msghdr));
            msg.msg_iov = (iovec*)buffers;
            msg.msg_iovlen = length;
            return ::sendmsg(m_sock,&msg,flags);
        }
        return -1;
    }

    int Socket::sendTo(const void *buffer, size_t length, const Address::ptr to, int flags) {
        if(isConnect()){
            return ::sendto(m_sock,buffer,length,flags,to->getAddr(),to->getAddrLen());
        }
        return -1;
    }

    int Socket::sendTo(const iovec *buffers, size_t length, const Address::ptr to, int flags) {
        if(isConnect()){
            msghdr msg;
            memset(&msg,0,sizeof (msghdr));
            msg.msg_iov = (iovec*)buffers;
            msg.msg_iovlen = length;
            msg.msg_name = to->getAddr();
            msg.msg_namelen = to->getAddrLen();
            return ::sendmsg(m_sock,&msg,flags);
        }
        return -1;
    }

    int Socket::recv(void *buffer, size_t length, int flags) {
        if(isConnect()) {
            return ::recv(m_sock, buffer, length, flags);
        }
        return -1;
    }

    int Socket::recv(iovec *buffers, size_t length, int flags) {
        if(isConnect()) {
            msghdr msg;
            memset(&msg, 0, sizeof(msg));
            msg.msg_iov = (iovec*)buffers;
            msg.msg_iovlen = length;
            return ::recvmsg(m_sock, &msg, flags);
        }
        return -1;
    }

    int Socket::recvFrom(void *buffer, size_t length, Address::ptr from, int flags) {
        if(isConnect()) {
            socklen_t len = from->getAddrLen();
            return ::recvfrom(m_sock, buffer, length, flags, from->getAddr(), &len);
        }
        return -1;
    }

    int Socket::recvFrom(iovec *buffers, size_t length, Address::ptr from, int flags) {
        if(isConnect()) {
            msghdr msg;
            memset(&msg, 0, sizeof(msg));
            msg.msg_iov = (iovec*)buffers;
            msg.msg_iovlen = length;
            msg.msg_name = from->getAddr();
            msg.msg_namelen = from->getAddrLen();
            return ::recvmsg(m_sock, &msg, flags);
        }
        return -1;
    }

    int Socket::getError() {
        int error = 0;
        socklen_t  len = sizeof (error);
        // ref: https://blog.csdn.net/wynter_/article/details/52600866
        // ref: https://linux.die.net/man/3/getsockopt
        if(!getsockopt(m_sock,SOL_SOCKET,SO_ERROR,&error,&len)){
            error = errno;
        }
        return error;
    }

    bool Socket::getOption(int level, int optname, void *optval, socklen_t *optlen) {

        if(getsockopt(m_sock,level,optname,optval,optlen)){
            WARN_LOG(g_logger)<<" Socket::getOption fail socket="
                              <<m_sock<<" level = "<<level<<" option="<<optname
                              <<" errno:"<<errno << "errstr = "<<strerror(errno);
            return false;
        }

        return true;
    }

    bool Socket::setOption(int level, int optname, const void *optval, socklen_t optlen) {

        if(setsockopt(m_sock,level,optname,optval,optlen)){
            WARN_LOG(g_logger)<<" Socket::setOption fail socket="
                              <<m_sock<<" level = "<<level<<" option="<<optname
                              <<" errno:"<<errno << "errstr = "<<strerror(errno);
            return false;
        }

        return true;
    }

    Socket::ptr Socket::CreateTCP( Address::ptr address) {
        return CWJ_CO_NET::Socket::ptr(new Socket(address->getFamily(),Type::TCP,0));
    }

    Socket::ptr Socket::CreateUDP(Address::ptr address) {
        return CWJ_CO_NET::Socket::ptr(new Socket(address->getFamily(),Type::UDP,0));
    }

    Socket::ptr Socket::CreateTCPSocket() {
        return CWJ_CO_NET::Socket::ptr(new Socket(Family::IPv4,Type::TCP,0));
    }

    Socket::ptr Socket::CreateUDPSocket() {
        return CWJ_CO_NET::Socket::ptr(new Socket(Family::IPv4,Type::UDP,0));
    }

    Socket::ptr Socket::CreateTCPSocket6() {
        return CWJ_CO_NET::Socket::ptr(new Socket(Family::IPv6,Type::TCP,0));
    }

    Socket::ptr Socket::CreateUDPSocket6() {
        return CWJ_CO_NET::Socket::ptr(new Socket(Family::IPv6,Type::UDP,0));
    }

    Socket::ptr Socket::CreateUnixTCPSocket() {
        return CWJ_CO_NET::Socket::ptr(new Socket(Family::UNIX,Type::TCP,0));
    }

    Socket::ptr Socket::CreateUnixUDPSocket() {
        return CWJ_CO_NET::Socket::ptr(new Socket(Family::UNIX,Type::UDP,0));
    }

    bool Socket::newSocket() {

        m_sock = socket(m_family,m_type,m_protocol);
        m_is_connect = false;
        if(m_sock == -1){
            ERROR_LOG(g_logger) << "create socket( family="<<m_family<<" ,type="<<m_type
                                <<" ,protocol="<<m_protocol<<") erron="<<errno<<"errstr="<<strerror(errno);
            return false;
        }else{
            this->init(m_sock,false);
            return true;
        }

    }

    bool Socket::init(int sock, bool is_connect) {
        m_is_connect = is_connect;
        m_sock = sock;
        int val = 1;
        // SO_REUSEADDR为了让处于time_wait的连接得到复用，需要始终该选项
        // ref: https://zhuanlan.zhihu.com/p/79999012
        setOption(SOL_SOCKET,SO_REUSEADDR,&val);
        // 禁用Nagle算法,减少小包的数量,让每条链接可同时有多个未确认的小包
        // ref: https://zhuanlan.zhihu.com/p/80104656
        if(m_type == SOCK_STREAM){
            setOption(IPPROTO_TCP,TCP_NODELAY,&val);
        }

        if(m_is_connect){
            getMRemoteAddr();
            getMLocalAddr();
        }

        return true;
    }

    const std::string Socket::toString() const {
        std::stringstream ss;
        ss << "[Socket sock=" << m_sock
           << " is_connected=" << m_is_connect
           << " family=" << m_family
           << " type=" << m_type
           << " protocol=" << m_protocol;
        if(m_local_addr) {
            ss << " local_address=" << m_local_addr->toString();
        }
        if(m_remote_addr) {
            ss << " remote_address=" << m_remote_addr->toString();
        }
        ss << "]";
        return ss.str();
    }

    void Socket::setNonBlock() {

        int flags=fcntl(m_sock,F_GETFL,0);
        flags |=O_NONBLOCK;

        if(fcntl(m_sock,F_SETFL,flags)){
            CWJ_ASSERT(false);
        }

        auto fd_ctx = FdMgr::GetInstance()->get(m_sock,true);
        fd_ctx->setMIsNonblock(true);
    }

    int64_t Socket::getSendTimeout() {
        auto fd_ctx = FdMgr::GetInstance()->get(m_sock,true);
        return fd_ctx->getTimeout(FdCtx::SNDTIMEO);
    }

    void Socket::setSendTimeout(int64_t v) {
        auto fd_ctx = FdMgr::GetInstance()->get(m_sock,true);
        fd_ctx->setTimeout(FdCtx::SNDTIMEO,v);

        struct timeval tv{int(v / 1000), int(v % 1000 * 1000)};
        setOption(SOL_SOCKET, SO_SNDTIMEO, &tv,sizeof(tv));
    }

    int64_t Socket::getRecvTimeout() {
        auto fd_ctx = FdMgr::GetInstance()->get(m_sock,true);
        return fd_ctx->getTimeout(FdCtx::RCVTIMEO);
    }

    void Socket::setRecvTimeout(int64_t v) {
        auto fd_ctx = FdMgr::GetInstance()->get(m_sock,true);
        fd_ctx->setTimeout(FdCtx::SNDTIMEO,v);

        struct timeval tv{int(v / 1000), int(v % 1000 * 1000)};
        setOption(SOL_SOCKET, SO_RCVTIMEO, &tv,sizeof(tv));
    }

    std::ostream& operator<<(std::ostream& os,const Socket& sock){
        return  os<<sock.toString();
    }

}