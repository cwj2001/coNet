#include "http_parser/http_parser.h"
#include "http/http_response_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

using namespace std;
using namespace CWJ_CO_NET;
using namespace CWJ_CO_NET::http;

static http_parser *parser;

int on_message_begin(http_parser* _) {
    (void)_;
    printf("\n***MESSAGE BEGIN***\n\n");
    return 0;
}

int on_headers_complete(http_parser* _) {
    (void)_;
    printf("\n***HEADERS COMPLETE***\n\n");
    return 0;
}

int on_message_complete(http_parser* _) {
    (void)_;
    printf("\n***MESSAGE COMPLETE***\n\n");
    return 0;
}

int on_url(http_parser* _, const char* at, size_t length) {
    (void)_;
    printf("Url: %.*s\n", (int)length, at);
    return 0;
}

int on_status(http_parser* _, const char* at, size_t length) {
    (void)_;
    printf("Status: %.*s\n", (int)length, at);
    return 0;
}

int on_header_field(http_parser* _, const char* at, size_t length) {
    (void)_;
    printf("Header field: %.*s\n", (int)length, at);
    return 0;
}

int on_header_value(http_parser* _, const char* at, size_t length) {
    (void)_;
    printf("Header value: %.*s\n", (int)length, at);
    return 0;
}

int on_body(http_parser* _, const char* at, size_t length) {
    (void)_;
    printf("Body: %.*s\n", (int)length, at);
    return 0;
}

int on_chunk_header(http_parser* _) {
    (void)_;
    printf("\n***CHUNK HEADER***\n\n");
    return 0;
}

int on_chunk_complete(http_parser* _) {
    (void)_;
    printf("\n***CHUNK COMPLETE***\n\n");
    return 0;
}

// http_parser的回调函数，需要获取HEADER后者BODY信息，可以在这里面处理
// 注意其中变量前面“.”表示的是当前结构体中的成员变量，
// 类似于对象.成员,同时可以可以乱序，如果未指定则必须要按原先的顺序
static http_parser_settings settings_null = {
        .on_message_begin = on_message_begin,	// 相当于settings_null.on_message_begin = on_message_begin,
        .on_url = on_url,
        .on_status = on_status,
        .on_header_field = on_header_field,
        .on_header_value = on_header_value,
        .on_headers_complete = on_headers_complete,
        .on_body = on_body,
        .on_message_complete = on_message_complete,
        .on_chunk_header = on_chunk_header,
        .on_chunk_complete = on_chunk_complete
};

int main(void)
{
    const char* buf;
    float start, end;
    size_t parsed;

    parser = (http_parser *)malloc(sizeof(http_parser));
    buf =  "GET http://admin.omsg.cn/uploadpic/2016121034000012.png HTTP/1.1\r\nHost: /"
           "admin.omsg.cn\r\nAccept:*/*\r\nConnection: Keep-Alive\r\n\r\n";

    start = (float)clock() / CLOCKS_PER_SEC;

    for (int i = 0; i < 1; i++) {
        // 初始化parser为Request类型
        http_parser_init(parser, HTTP_REQUEST);
        // 执行解析过程
        parsed = http_parser_execute(parser, &settings_null, buf, strlen(buf));
    }

    end = (float)clock() / CLOCKS_PER_SEC;

    buf = "HTTP/1.1 200 OK\r\n"
          "Date: Tue, 04 Aug 2009 07:59:32 GMT\r\n"
          "Server: Apache\r\n"
          "X-Powered-By: Servlet/2.5 JSP/2.1\r\n"
          "Content-Type: text/xml; charset=utf-8\r\n"
          "Connection: close\r\n"
          "\r\n"
          "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
          "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\">\n"
          "  <SOAP-ENV:Body>\n"
          "    <SOAP-ENV:Fault>\n"
          "       <faultcode>SOAP-ENV:Client</faultcode>\n"
          "       <faultstring>Client Error</faultstring>\n"
          "    </SOAP-ENV:Fault>\n"
          "  </SOAP-ENV:Body>\n"
          "</SOAP-ENV:Envelope>";

    // 初始化parser为Response
    http_parser_init(parser, HTTP_RESPONSE);
    // 执行解析过程
    parsed = http_parser_execute(parser, &settings_null, buf, strlen(buf));

    free(parser);
    parser = NULL;

    printf("Elapsed %f seconds.\n", (end - start));


    cout<<"----------------------------------------"<<endl;

    HttpResponseParser parser1;

    parser1.execute(buf,strlen(buf));

//    parser1.getData()->dump(cout);

    return 0;

}