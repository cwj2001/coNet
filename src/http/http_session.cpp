//
// Created by 抑~风 on 2023-03-02.
//
#include <sys/types.h>
#include <sys/socket.h>
#include <memory>
#include <sstream>
#include "http_session.h"
#include "log.h"
#include "macro.h"

namespace CWJ_CO_NET{
    namespace http{

        static auto g_logger = GET_LOGGER("http_server");

        HttpSession::HttpSession(const Socket::ptr &sock,bool is_owner, int buf_size)
                                :m_sock(sock),m_buf(buf_size),m_pre_buf_size(buf_size),m_is_owner(is_owner) {
        }

        std::pair<HttpRequest::ptr,bool> HttpSession::recvRequest() {


            while(!m_req_parser.hasNext()){
                std::vector<iovec> buffers;
                m_buf.getWriteBuffers(buffers,m_pre_buf_size,false);
                auto offset = m_sock->recv(&buffers[0],buffers.size());

                if(offset<=0) {
//                    CWJ_ASSERT(false);
                    return {std::make_shared<HttpRequest>(),false};
                };
                buffers.clear();
                m_buf.getWriteBuffers(buffers,offset,false);
                for(auto& a : buffers)  m_req_parser.execute((char*)a.iov_base,a.iov_len);
                m_buf.updateNullWrite(offset);
            };
//            CWJ_ASSERT(false);
            return m_req_parser.getNextData();
        }

        int HttpSession::sendResponse(HttpResponse::ptr rsp) {
            std::stringstream ss;
            rsp->dump(ss);
            auto s = ss.str();
            auto offset = m_sock->send(s.c_str(),s.size());
            return offset;
        }

        HttpSession::~HttpSession() {
            if(m_is_owner)  m_sock->close();
        }
    }
}