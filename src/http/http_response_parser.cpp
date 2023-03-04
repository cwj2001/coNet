//
// Created by 抑~风 on 2023-03-01.
//

#include <cstring>
#include "log.h"
#include "http_response_parser.h"
#include "macro.h"
namespace CWJ_CO_NET {
    namespace http {

        static auto g_logger = GET_LOGGER("http_server");

        HttpResponseParser::HttpResponseParser() {
            http_parser_init(&m_parser, HTTP_RESPONSE);
            m_parser.data = this;
        }

        const http_parser_settings& HttpResponseParser::getSetting() {
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

        int HttpResponseParser::on_message_begin(http_parser* parser) {
            INFO_LOG(g_logger)<<("******** message_begin ********");
            auto t = (HttpResponseParser*)parser->data;
            initData(t);
            return 0;
        }

        void HttpResponseParser::initData(HttpResponseParser *t) {
            t->m_resp_que.push({std::make_shared<HttpResponse>(),false});
            t->m_last_key = "";
        }


        int HttpResponseParser::on_headers_complete(http_parser* _) {
            (void)_;
            INFO_LOG(g_logger)<<("\n***HEADERS COMPLETE***\n\n");
            return 0;
        }

        int HttpResponseParser::on_url(http_parser* parser, const char* at, size_t length) {
            auto t = (HttpResponseParser*)parser->data;
            return 0;
        }

#define XX(code,state,str) \
        if(StrCmpIg(#str,cs,true)){ \
                t->m_resp_que.back().first->setStatus(HttpStatus::state);   \
        }

        int HttpResponseParser::on_status(http_parser* parser, const char* at, size_t length) {
            INFO_LOG(g_logger) << std::string(at,length) <<" "<<strlen(at);
            std::string str(at,length);
            auto cs = str.c_str();
            auto t = (HttpResponseParser*)parser;
            HTTP_STATUS_MAP(XX);

            return 0;
        }

#undef XX

        int HttpResponseParser::on_message_complete(http_parser* parser) {
            INFO_LOG(g_logger)<<("******** message_complete ********");
            HttpResponseParser* self = (HttpResponseParser*)parser->data;
            auto t = self->m_resp_que.back();
            self->m_finish ++;
            t.second = parser->http_errno ? false : true;
            return 0;
        }

        int HttpResponseParser::on_header_field(http_parser* parser, const char* at, size_t length) {
            auto t = (HttpResponseParser*)parser;
            t->m_last_key = std::string(at,length);
            return 0;
        }

        int HttpResponseParser::on_header_value(http_parser* parser, const char* at, size_t length) {
            auto t = (HttpResponseParser*)parser;
            t->m_resp_que.back().first->setHeader(t->m_last_key,std::string(at,length));
            return 0;
        }

        int HttpResponseParser::on_body(http_parser* parser, const char* at, size_t length) {
//            printf("Body: %.*s\n", (int)length, at);
            auto t = (HttpResponseParser*)parser->data;
            t->m_resp_que.back().first->setBody(std::string(at,length));
            return 0;
        }

        int HttpResponseParser::on_chunk_header(http_parser* _) {
            (void)_;
            DEBUG_LOG(g_logger)<<("\n***CHUNK HEADER***\n\n");
            return 0;
        }

        int HttpResponseParser::on_chunk_complete(http_parser* _) {
            (void)_;
            DEBUG_LOG(g_logger)<<("\n***CHUNK COMPLETE***\n\n");
            return 0;
        }

        size_t HttpResponseParser::execute(const char *data, size_t len) {
//            if(m_finish)  http_parser_init(&m_parser,HTTP_RESPONSE);
            auto offset = http_parser_execute(&m_parser,&getSetting(),data,len);
//            memmove(data,data+offset,len-offset);
            return offset;
        }

        uint64_t HttpResponseParser::GetHttpRequestBufferSize() {
            return 0;
        }

        uint64_t HttpResponseParser::GetHttpRequestMaxBodySize() {
            return 0;
        }

    }
}

