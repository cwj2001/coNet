//
// Created by 抑~风 on 2023-03-01.
//

#ifndef CWJ_CO_NET_HTTP_REQUEST_PARSER_H
#define CWJ_CO_NET_HTTP_REQUEST_PARSER_H

#include "http_parser/http_parser.h"
#include "http.h"
namespace CWJ_CO_NET { namespace http{

    class HttpRequestParser{
    public:
        HttpRequestParser();

        size_t execute(const char* data, size_t len);

        int isFinished();

        int hasError();

        HttpRequest::ptr getData() const { return m_req;}

        void setError(int v) { m_error = v;}

        uint64_t getContentLength();

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
        HttpRequest::ptr m_req;
        std::string m_last_key;
        int m_error = 0;
        bool m_finish = false;

        static void initData(HttpRequestParser *t);
    };


} }
#endif //CWJ_CO_NET_HTTP_REQUEST_PARSER_H
