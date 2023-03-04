//
// Created by 抑~风 on 2023-03-02.
//

#include "socket.h"
#include "byteArray.h"
#include "tcpserver.h"
#include "http_server.h"

#include "http_session.h"

namespace CWJ_CO_NET{
    namespace http{

        static auto g_logger = GET_LOGGER("http_server");

        void HttpServer::handleClient(Socket::ptr sock) {
            HttpSession::ptr session = std::make_shared<HttpSession>(sock,true);
            bool keepalive = true;
            while(sock->isConnect() && keepalive){
                auto req = session->recvRequest();
                auto t = req.first;
                CWJ_ASSERT(req.first);

                std::stringstream ss;
                req.first->dump(ss);
                INFO_LOG(g_logger) << "resv : " << ss.str();



                HttpResponse::ptr resp = std::make_shared<HttpResponse>(req.first->getVersion(),!keepalive);
                resp->setHeader("server","cwj_co_net");

                if(t->getHeader("connection","keep-alive") == "close"){
                    keepalive = false;
                    resp->setHeader("connection","close");
                    resp->setClose(true);
                }



                if(!req.second || !req.first)  {
                    recv_error(req.first,resp);
                    sock->close();
                    continue;
                }
                else {
                    m_dispatch.handle(req.first,resp);

                    ss.clear();
                    resp->dump(ss);
                    INFO_LOG(g_logger) << "send : "<<ss.str();

                    if(session->sendResponse(resp) <= 0)    sock->close();
                }

            }

            // 套接字关闭前的回调
            sock->close();
            uint64_t u = 2;
            write(m_accept_error_wake_fd,&u,sizeof(uint64_t));
//            INFO_LOG(g_logger) << "sock: "<<sock->getSocket()<<" close ";

        }

        void HttpServer::handleAcceptError() {
            uint64_t u = 0;
            m_accept_idle_count++;
            read(m_accept_error_wake_fd,&u,sizeof(u));
            INFO_LOG(g_logger) << "accept_wake_read="<<u;
            m_accept_idle_count--;
        }

        void HttpServer::recv_error(HttpRequest::ptr req, HttpResponse::ptr resp) {
            WARN_LOG(g_logger) << "recv request error";
        }

        HttpServer::HttpServer(const std::string &mName, size_t acceptThreadCount, size_t ioThreadCount,
                               bool acceptShared, const ServletDispatch &mDispatch)
                               : TcpServer(mName,acceptThreadCount,ioThreadCount,acceptShared)
                               ,m_dispatch(mDispatch),m_accept_idle_count(0) {

            m_accept_error_wake_fd = eventfd(0,0);

        }
    }
}