//
// Created by 抑~风 on 2023/2/5.
//
#include <string>
#include <cstring>

#include "tcpserver.h"
#include "log.h"
#include "config.h"
namespace CWJ_CO_NET {

    static auto g_logger = GET_LOGGER("tcpserver");
    static auto g_more_tcp_server_recv_buffer_size = SingleConfigMgr::GetInstance()->lookup<int>("moretcpserver.recv_buffer_size",1024,"more_tcp_server_recv_buffer_size");
    static auto g_more_tcp_server_bytearray_base_size = SingleConfigMgr::GetInstance()->lookup<int>("moretcpserver.bytearray_base_size",1024,"more_tcp_server_bytearray_base_size");
    static auto g_more_tcp_server_error_sleep = SingleConfigMgr::GetInstance()->lookup<int>("moretcpserver.error_sleep",3,"more_tcp_server_error_sleep");

    void TcpServer::bind(const std::vector<Address::ptr> &addrs, std::vector<Address::ptr> &fails) {
        for (auto &a:addrs) {
            auto sock = Socket::CreateTCPSocket();
            if (!sock->bind(a)) {
                WARN_LOG(g_logger) << "tcpserver bind addr:" << *a << " fail ";
                fails.push_back(a);
            }
            if (!sock->listen()) {
                WARN_LOG(g_logger) << "tcpserver sock:" << *sock << " listen" << " fail ";
                fails.push_back(a);
            }
            m_accept_fds.push_back(sock);
        }
    }

    bool TcpServer::bind(Address::ptr addr) {
        std::vector<Address::ptr> fails;
        bind({addr}, fails);
        return fails.empty();
    }

    void TcpServer::start() {
        {
            MutexType::Lock lock(m_mutex);
            if (!m_stopping) {
                return;
            }
            m_stopping = false;
        }
        CWJ_ASSERT(m_accept_fds.size());
        for (auto a :m_accept_fds) {
            a->setNonBlock();
            m_accept_iom->schedule(std::bind(&TcpServer::handleAccept, shared_from_this(), a), -1);
        }
        m_io_iom->start();
        m_accept_iom->start();
    }

    void TcpServer::stop() {
        {
            MutexType::Lock lock(m_mutex);
            if (m_stopping) {
                return;
            }
            m_stopping = true;
        }

        auto self = IOManager::GetThis();
        m_accept_iom->auto_stop();
        m_io_iom->auto_stop();

        // 其原因是为了触发accept,因为hook accept实现保证了，其除非出错了，
        // 不然就一定会正常返回有效的fd,使用cancelAll无法使其跳出accept,所以需要自己手动连接，触发accept,使其跳出循环
        // 然后再scheduler中的调度函数中退出当前程序，所以下面循环也必须放置到auto_stop后面；
        auto sock = Socket::CreateTCPSocket();
        for (auto a : m_accept_fds) {
            sock->connect(a->getMLocalAddr());
        }
        sock->close();
    }

    void TcpServer::handleClient(Socket::ptr sock) {
        INFO_LOG(g_logger) << "handleClient  sock:"<< *sock;
        sock->close();
        this->stop();
        INFO_LOG(g_logger) << "handleClient end";
    }

    void TcpServer::handleAccept(Socket::ptr sock) {
        while (!m_stopping) {
            auto client = sock->accept();

            if(!client) {
                ERROR_LOG(g_logger) << "accept errno=" << errno
                                    << " errstr=" << strerror(errno);
                handleAcceptError();
                continue;
            }else{
                INFO_LOG(g_logger) << "accept accrpt";
            }
            if((m_accept_shared && m_io_iom->getMPendingEventCount() > m_accept_iom->getMPendingEventCount())
                || m_only_accept){
                INFO_LOG(g_logger) << "1.add client";
                m_accept_iom->schedule(std::bind(&TcpServer::handleClient, shared_from_this(), client),-1);
            }else {
                INFO_LOG(g_logger) << "2.add client";
                m_io_iom->schedule(std::bind(&TcpServer::handleClient, shared_from_this(), client), -1);
            }
        }
    }

    TcpServer::TcpServer(const std::string &mName, size_t accept_thread_count, size_t io_thread_count,
                         bool accept_shared)
            : m_name(mName)
            , m_accept_iom(new IOManager(accept_thread_count
                                            , true, mName + ".accept_iom"))
            , m_io_iom(new IOManager(io_thread_count
                                        , false, mName + ".io_iom"))
            , m_accept_shared(accept_shared)
            , m_stopping(true)
            , m_only_accept(!io_thread_count){}

    void TcpServer::handleAcceptError() {
        sleep(g_more_tcp_server_error_sleep->getMVal());
    }


    void MoreTcpServer::handleClient(Socket::ptr sock) {
        // 刚连接时的回调
        onConnection(sock);
        auto base_size = g_more_tcp_server_bytearray_base_size->getMVal();
        ByteArray::ptr recv_buffer(new ByteArray(base_size));
        ByteArray::ptr send_buffer(new ByteArray(base_size));

        int size = 0;
        std::vector<iovec> recv_ios, send_ios;

        while (sock->isConnect()) {

            // 接收报文
            recv_ios.clear();
            recv_buffer->getWriteBuffers(recv_ios, g_more_tcp_server_recv_buffer_size->getMVal(),false);
            if((size = sock->recv(&*recv_ios.begin(), recv_ios.size()))<=0) {
                break;
            }
            recv_buffer->updateNullWrite(size);
            // 处理报文
            onMessage(sock,recv_buffer, send_buffer);

            // 发送报文
            bool is_send = false;
            while (send_buffer->getMDataSize() > 0) {
                send_ios.clear();
                send_buffer->getReadBuffers(send_ios);
                sock->send(&*send_ios.begin(), send_ios.size());
                is_send = true;
            }
            // 报文发送完成的回调
            if(is_send) onWriteComplete(sock);

        }
        // 套接字关闭前的回调
        onClose(sock);
        sock->close();
        uint64_t u = 2;
        write(m_accept_error_wake_fd,&u,sizeof(uint64_t));
    }

    void MoreTcpServer::handleAcceptError() {
        uint64_t u = 0;
        m_accept_idle_count++;
        read(m_accept_error_wake_fd,&u,sizeof(u));
        INFO_LOG(g_logger) << "accept_wake_read="<<u;
        m_accept_idle_count--;
    }

    MoreTcpServer::MoreTcpServer(const std::string &mName, size_t acceptThreadCount, size_t ioThreadCount,
                                 bool acceptShared) :m_accept_idle_count(0), TcpServer(mName, acceptThreadCount, ioThreadCount, acceptShared) {

        m_accept_error_wake_fd = eventfd(0,0);

    }

    MultiServer::MultiServer(const std::string &mName, size_t accept_thread_count, size_t io_thread_count,
                             bool accept_shared) {

    }

    void MultiServer::bind(const std::vector<Address::ptr> &addrs, std::vector<Address::ptr> &fails) {

    }

    bool MultiServer::bind(Address::ptr addr) {
        return false;
    }

    void MultiServer::start() {

    }

    void MultiServer::stop() {

    }

    void MultiServer::handleClient(Socket::ptr sock) {

    }

    void MultiServer::handleAccept(Socket::ptr sock) {

    }

    void MultiServer::handleAcceptError() {

    }
}