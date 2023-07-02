//
// Created by 抑~风 on 2023-03-02.
//

#ifndef CWJ_CO_NET_HTTP_SESSION_H
#define CWJ_CO_NET_HTTP_SESSION_H

#include <memory>
#include <string>
#include <queue>
#include "socket.h"
#include "http/http.h"
#include "http/http_request_parser.h"
#include "http/http_response_parser.h"
#include "byteArray.h"
namespace CWJ_CO_NET{
    namespace http{

        class HttpSession{
        public:
            using ptr = std::shared_ptr<HttpSession>;

            HttpSession(const Socket::ptr &sock,bool is_owner = true,int buf_size = 1024);

            // bool 是否成功
            std::pair<HttpRequest::ptr,bool> recvRequest();

            // 返回值发送字节数：>0 正常 =0 关闭 <= sock

            int sendResponse(HttpResponse::ptr rsp);

            virtual ~HttpSession();

        private:
            Socket::ptr m_sock;
            HttpRequestParser m_req_parser;
            HttpResponseParser m_resp_parser;
            ByteArray m_buf;
            size_t m_pre_buf_size = 1024;
            bool m_is_owner ;
        };


    }
}

#endif //CWJ_CO_NET_HTTP_SESSION_H
