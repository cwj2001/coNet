//
// Created by 抑~风 on 2023-03-02.
//

#ifndef CWJ_CO_NET_HTTP_SERVER_H
#define CWJ_CO_NET_HTTP_SERVER_H

#include <memory>

#include "tcpserver.h"
#include "socket.h"
#include "byteArray.h"
#include "http_servlet.h"

namespace CWJ_CO_NET{
    namespace http{
        class HttpServer : public TcpServer{
        public:
            using ptr = std::shared_ptr<HttpServer>;
        public:
            HttpServer(const std::string &mName, size_t acceptThreadCount, size_t ioThreadCount, bool acceptShared,
                       const ServletDispatch &mDispatch);

            void handleClient(Socket::ptr sock) override;
           void handleAcceptError() override;
           void recv_error(HttpRequest::ptr req,HttpResponse::ptr resp);
        private:
            ServletDispatch m_dispatch;
            eventfd_t m_accept_error_wake_fd;
            std::atomic<size_t> m_accept_idle_count;
        };
    }
}

#endif //CWJ_CO_NET_HTTP_SERVER_H
