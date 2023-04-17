//
// Created by 抑~风 on 2023/2/5.
//

#ifndef CWJ_CO_NET_TCPSERVER_H
#define CWJ_CO_NET_TCPSERVER_H

#include <memory>
#include <string>
#include <vector>
#include <atomic>
#include <sys/eventfd.h>

#include "cwj_process_cycle.h"
#include "iomanager.h"
#include "address.h"
#include "socket.h"
#include "mutex.h"
#include "byteArray.h"

namespace CWJ_CO_NET {

    class TcpServer : public std::enable_shared_from_this<TcpServer> {
    public:
        using ptr = std::shared_ptr<TcpServer>;
        using MutexType = Mutex;

        TcpServer(const std::string &mName, size_t accept_thread_count, size_t io_thread_count, bool accept_shared);

        virtual ~TcpServer() {};

        void bind(const std::vector<Address::ptr> &addrs, std::vector<Address::ptr> &fails);

        bool bind(Address::ptr addr);

        void start();

        void stop();

    protected:

        virtual void handleClient(Socket::ptr sock);

        virtual void handleAccept(Socket::ptr sock);

        virtual void handleAcceptError();

    private:
        const std::string m_name;
        IOManager::ptr m_io_iom;
        IOManager::ptr m_accept_iom;
        // 还差一个专门处理耗时任务的线程池
        std::vector<Socket::ptr> m_accept_fds;

        std::atomic<bool> m_accept_shared{false};
        std::atomic<bool> m_stopping{true};
        std::atomic<bool>m_only_accept{true};

        MutexType m_mutex;
    };


    class MoreTcpServer : public TcpServer {
    public:
        using ptr = std::shared_ptr<MoreTcpServer>;
        using CallBack = std::function<void()>;

        MoreTcpServer(const std::string &mName, size_t acceptThreadCount, size_t ioThreadCount, bool acceptShared);

        virtual void onConnection(Socket::ptr&) = 0;

        virtual void onClose(Socket::ptr&) = 0;

        virtual void onWriteComplete(Socket::ptr&) = 0;

        virtual void onMessage(Socket::ptr& sock,ByteArray::ptr& recv_buffer, ByteArray::ptr& send_buffer) = 0;

        void handleClient(Socket::ptr sock) override;

        void handleAcceptError() override;

    private:
        eventfd_t m_accept_error_wake_fd;
        std::atomic<size_t> m_accept_idle_count;// 用于负载均衡判断阻塞的accept数目的多少
    };

    class SingleServer{
    public:
        using ptr = std::shared_ptr<SingleServer>;
    private:
    };

    class MultiServer{
    public:
        using ptr = std::shared_ptr<MultiServer>;

        using MutexType = Mutex;
        MultiServer(const std::string &mName, size_t accept_thread_count, size_t io_thread_count, bool accept_shared);

        virtual ~MultiServer() {};

        void bind(const std::vector<Address::ptr> &addrs, std::vector<Address::ptr> &fails);

        bool bind(Address::ptr addr);

        void start();

        void stop();

    protected:

        virtual void handleClient(Socket::ptr sock);

        virtual void handleAccept(Socket::ptr sock);

        virtual void handleAcceptError();


    private:

        MasterProcess::ptr masterProcess;

        const std::string m_name;
        // 还差一个专门处理耗时任务的线程池
        std::vector<Socket::ptr> m_accept_fds;

        std::atomic<bool> m_accept_shared{false};
        std::atomic<bool> m_stopping{true};
        std::atomic<bool>m_only_accept{true};

        MutexType m_mutex;


    };


}

#endif //CWJ_CO_NET_TCPSERVER_H
