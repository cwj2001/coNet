//
// Created by 抑~风 on 2023-03-01.
//
#include <cstring>
#include "http_request_parser.h"
#include "macro.h"
#include "log.h"

namespace CWJ_CO_NET {
    namespace http {

        static auto g_logger = GET_LOGGER("http_server");

        http::HttpRequestParser::HttpRequestParser() {
            http_parser_init(&m_parser, HTTP_REQUEST);
            m_parser.data = this;
        }

        const http_parser_settings& HttpRequestParser::getSetting() {
            static http_parser_settings settings_null = {
                    .on_message_begin = on_message_begin,    // 相当于settings_null.on_message_begin = on_message_begin,
                    .on_url = on_url,
                    .on_status = on_status,
                    .on_header_field = on_header_field,
                    .on_header_value = on_header_value,
                    .on_headers_complete = on_headers_complete,
                    .on_body = on_body,
                    .on_message_complete = on_message_complete,
                    .on_chunk_header = on_chunk_header,
                    .on_chunk_complete = on_chunk_complete,
            };
            return settings_null;
        }

        int HttpRequestParser::on_message_begin(http_parser* parser) {
            auto t = (HttpRequestParser*)parser->data;
            initData(t);
            return 0;
        }

        void HttpRequestParser::initData(HttpRequestParser *t) {
            t->m_req_que.push({std::make_shared<HttpRequest>(0x11,false),false});
            t->m_last_key = "";
        }

        int HttpRequestParser::on_headers_complete(http_parser* _) {
            (void)_;
            INFO_LOG(g_logger)<<"\n***HEADERS COMPLETE***\n";
            return 0;
        }

        int HttpRequestParser::on_url(http_parser* parser, const char* at, size_t length) {
//            INFO_LOG(g_logger) << std::string(at,length)<<std::endl;
            auto t = (HttpRequestParser*)parser->data;
            t->m_req_que.back().first->setPath(std::string(at,length));
            return 0;
        }

        int HttpRequestParser::on_status(http_parser* parser, const char* at, size_t length) {
            auto t = (HttpRequestParser*)parser;
            return 0;
        }

        int HttpRequestParser::on_message_complete(http_parser* parser) {
            HttpRequestParser* self = (HttpRequestParser*)parser->data;
            auto& req = self->m_req_que.back();
            self->m_finish ++ ;
            req.second = !parser->http_errno;
            req.first->setMethod(HttpMethod(parser->method));
            return 0;
        }

        int HttpRequestParser::on_header_field(http_parser* parser, const char* at, size_t length) {
            auto t = (HttpRequestParser*)parser;
            t->m_last_key = std::string(at,length);
            return 0;
        }

        int HttpRequestParser::on_header_value(http_parser* parser, const char* at, size_t length) {
            auto t = (HttpRequestParser*)parser;
            t->m_req_que.back().first->setHeader(t->m_last_key,std::string(at,length));
            return 0;
        }

        int HttpRequestParser::on_body(http_parser* parser, const char* at, size_t length) {
            INFO_LOG(g_logger)<<("Body: %.*s\n", (int)length, at);
            auto t = (HttpRequestParser*)parser->data;
            t->m_req_que.back().first->setBody(std::string(at,length));
            return 0;
        }

        int HttpRequestParser::on_chunk_header(http_parser* _) {
            (void)_;
            INFO_LOG(g_logger)<<("\n***CHUNK HEADER***\n\n");
            return 0;
        }

        int HttpRequestParser::on_chunk_complete(http_parser* _) {
            (void)_;
            INFO_LOG(g_logger)<<("\n***CHUNK COMPLETE***\n\n");
            return 0;
        }

        size_t HttpRequestParser::execute(const char *data, size_t len) {
            INFO_LOG(g_logger)<<"init_data:"<<std::string(data,len);
            auto offset = http_parser_execute(&m_parser,&getSetting(),data,len);
//            memmove(data,data+offset,len-offset);
//            if(m_req_que.size())    INFO_LOG(g_logger) << m_req_que.front().first->toString();
            return offset;
        }

        int HttpRequestParser::hasNext() {
            return m_finish > 0;
        }

        uint64_t HttpRequestParser::GetHttpRequestBufferSize() {
            return 0;
        }

        uint64_t HttpRequestParser::GetHttpRequestMaxBodySize() {
            return 0;
        }
    }
}
