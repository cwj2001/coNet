//
// Created by 抑~风 on 2023-03-01.
//
#include <cstring>
#include "http_request_parser.h"

namespace CWJ_CO_NET {
    namespace http {

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
            t->m_req.reset(new HttpRequest);
            t->m_last_key = "";
            t->m_error = 0;
            t->m_finish = false;
        }

        int HttpRequestParser::on_headers_complete(http_parser* _) {
            (void)_;
            printf("\n***HEADERS COMPLETE***\n\n");
            return 0;
        }

        int HttpRequestParser::on_url(http_parser* parser, const char* at, size_t length) {
            auto t = (HttpRequestParser*)parser->data;
            t->m_req->setPath(std::string(at,length));
            return 0;
        }

        int HttpRequestParser::on_status(http_parser* parser, const char* at, size_t length) {
            auto t = (HttpRequestParser*)parser;
            return 0;
        }

        int HttpRequestParser::on_message_complete(http_parser* parser) {
            HttpRequestParser* self = (HttpRequestParser*)parser->data;
            self->m_finish = true;
            self->m_error = parser->http_errno;
            self->m_req->setMethod(HttpMethod(parser->method));
            http_parser_init(parser,HTTP_REQUEST);
            return 0;
        }

        int HttpRequestParser::on_header_field(http_parser* parser, const char* at, size_t length) {
            auto t = (HttpRequestParser*)parser;
            t->m_last_key = std::string(at,length);
            return 0;
        }

        int HttpRequestParser::on_header_value(http_parser* parser, const char* at, size_t length) {
            auto t = (HttpRequestParser*)parser;
            t->m_req->setHeader(t->m_last_key,std::string(at,length));
            return 0;
        }

        int HttpRequestParser::on_body(http_parser* parser, const char* at, size_t length) {
            printf("Body: %.*s\n", (int)length, at);
            auto t = (HttpRequestParser*)parser->data;
            t->m_req->setBody(std::string(at,length));
            return 0;
        }

        int HttpRequestParser::on_chunk_header(http_parser* _) {
            (void)_;
            printf("\n***CHUNK HEADER***\n\n");
            return 0;
        }

        int HttpRequestParser::on_chunk_complete(http_parser* _) {
            (void)_;
            printf("\n***CHUNK COMPLETE***\n\n");
            return 0;
        }

        size_t HttpRequestParser::execute(const char *data, size_t len) {
            auto offset = http_parser_execute(&m_parser,&getSetting(),data,len);
//            memmove(data,data+offset,len-offset);
            return offset;
        }

        int HttpRequestParser::isFinished() {
            return m_finish;
        }

        int HttpRequestParser::hasError() {
            return m_error;
        }

        uint64_t HttpRequestParser::getContentLength() {
            return std::stoll(m_req->getHeader("content-length", 0));
        }

        uint64_t HttpRequestParser::GetHttpRequestBufferSize() {
            return 0;
        }

        uint64_t HttpRequestParser::GetHttpRequestMaxBodySize() {
            return 0;
        }
    }
}
