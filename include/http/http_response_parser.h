//
// Created by 抑~风 on 2023-03-01.
//

#ifndef CWJ_CO_NET_HTTP_RESPONSE_PARSER_H
#define CWJ_CO_NET_HTTP_RESPONSE_PARSER_H
#include <queue>
#include "http_parser/http_parser.h"
#include "http.h"
namespace CWJ_CO_NET {
    namespace http {

        class HttpResponseParser {
        public:
            HttpResponseParser();

            size_t execute(const char *data, size_t len);

            int hasNext(){
                return m_finish > 0;
            };


            std::pair<HttpResponse::ptr,bool>getNextData(){
                if(!m_finish)   return {nullptr,false};
                auto t = m_resp_que.front();
                m_resp_que.pop();
                m_finish -= 1;
                return t;
            }

            const http_parser &getParser() const { return m_parser; }

        public:
            static uint64_t GetHttpRequestBufferSize();

            static uint64_t GetHttpRequestMaxBodySize();

        private:
            static int on_message_begin(http_parser *parser);

            static int on_headers_complete(http_parser *_);

            static int on_message_complete(http_parser *parser);

            static int on_url(http_parser *parser, const char *at, size_t length);

            static int on_status(http_parser *parser, const char *at, size_t length);

            static int on_header_field(http_parser *parser, const char *at, size_t length);

            static int on_header_value(http_parser *parser, const char *at, size_t length);

            static int on_body(http_parser *parser, const char *at, size_t length);

            static int on_chunk_header(http_parser *_);

            static int on_chunk_complete(http_parser *_);

        public:
            const http_parser_settings &getSetting();

        private:
            http_parser m_parser;
            std::queue<std::pair<HttpResponse::ptr,bool>> m_resp_que;
            std::string m_last_key;
            uint32_t m_finish = 0;
            static void initData(HttpResponseParser *t);
        };

    }
}

#endif //CWJ_CO_NET_HTTP_RESPONSE_PARSER_H