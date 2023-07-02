//
// Created by 抑~风 on 2023-03-01.
//

#ifndef CWJ_CO_NET_HTTP_REQUEST_PARSER_H
#define CWJ_CO_NET_HTTP_REQUEST_PARSER_H

#include <queue>
#include "http_parser/http_parser.h"
#include "http.h"
#include "log.h"
namespace CWJ_CO_NET { namespace http{

    class HttpRequestParser{
    public:
        HttpRequestParser();

        size_t execute(const char* data, size_t len);

        int hasNext();


        std::pair<HttpRequest::ptr,bool> getNextData(){
            if(m_finish <= 0)   return {nullptr,false};
            auto t = m_req_que.front();
            m_req_que.pop();
            m_finish -= 1;
            INFO_LOG(GET_ROOT_LOGGER()) << "m_finish:" << m_finish;
            return t;
        }

        const http_parser& getParser() const { return m_parser;}
    public:
        static uint64_t GetHttpRequestBufferSize();

        static uint64_t GetHttpRequestMaxBodySize();

    private:
        static int on_message_begin(http_parser* parser);

        static int on_headers_complete(http_parser* _);

        static int on_message_complete(http_parser* parser);

        static int on_url(http_parser* parser, const char* at, size_t length) ;

        static int on_status(http_parser* parser, const char* at, size_t length) ;

        static int on_header_field(http_parser* parser, const char* at, size_t length);

        static int on_header_value(http_parser* parser, const char* at, size_t length);

        static int on_body(http_parser* parser, const char* at, size_t length) ;

        static int on_chunk_header(http_parser* _);

        static int on_chunk_complete(http_parser* _);

        const http_parser_settings& getSetting();
    private:
        http_parser m_parser;
        // 协议报文和错误码
        std::queue<std::pair<HttpRequest::ptr,bool>> m_req_que;
        std::string m_last_key;
        uint32_t m_finish = 0;

        static void initData(HttpRequestParser *t);
    };


} }
#endif //CWJ_CO_NET_HTTP_REQUEST_PARSER_H
