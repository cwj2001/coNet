//
// Created by 抑~风 on 2023/1/30.
//

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include "http/http_response_parser.h"
#include "http_parser/http_parser.h"
#include "util.h"
#include "log.h"

using namespace std;
using namespace CWJ_CO_NET;
using namespace CWJ_CO_NET::http;

static auto g_logger = GET_ROOT_LOGGER();

int main() {
    auto buf_1 = "HTTP/1.1 200 OK\n"
                 "Access-Control-Allow-Credentials: true\n"
                 "Access-Control-Allow-Headers: Content-Type\n"
                 "Access-Control-Allow-Methods: POST, GET\n"
                 "Access-Control-Allow-Origin: https://www.baidu.com\n"
                 "Content-Length: 0\n"
                 "Date: Wed, 01 Mar 2023 14:49:22 GMT\n"
                 "Tracecode: 59126821620555210060389211402030110\n"
                 "Content-Type: text/plain; charset=utf-8\r\n\r\n";

    auto buf_2 = "HTTP/1.1 200 OK\r\n"
          "Date: Tue, 04 Aug 2009 07:59:32 GMT\r\n"
          "Server: Apache\r\n"
          "X-Powered-By: Servlet/2.5 JSP/2.1\r\n"
          "Content-Type: text/xml; charset=utf-8\r\n"
          "Connection: close\r\n"
          "content-length: 312\r\n"
          "\r\n"
          "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
          "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\">\n"
          "  <SOAP-ENV:Body>\n"
          "    <SOAP-ENV:Fault>\n"
          "       <faultcode>SOAP-ENV:Client</faultcode>\n"
          "       <faultstring>Client Error</faultstring>\n"
          "    </SOAP-ENV:Fault>\n"
          "  </SOAP-ENV:Body>\n"
          "</SOAP-ENV:Envelope>\n"
         "HTTP/1.1 200 OK\n"
         "Access-Control-Allow-Credentials: true\n"
         "Access-Control-Allow-Headers: Content-Type\n"
         "Access-Control-Allow-Methods: POST, GET\n"
         "Access-Control-Allow-Origin: https://www.baidu.com\n"
         "Content-Length: 0\n"
         "Date: Wed, 01 Mar 2023 14:49:22 GMT\n"
         "Tracecode: 59126821620555210060389211402030110\n"
         "Content-Type: text/plain; charset=utf-8\r\n\r\n";;
            ;


    HttpResponseParser parser;
//
    cout << "1---------------------" << endl;

    parser.execute(buf_1, strlen(buf_1));

    cout << "2---------------------" << endl;

    cout<<parser.execute(buf_2, strlen(buf_2))<<endl;

    while(parser.hasNext()){
        INFO_LOG(g_logger) <<" ----------------- ";
        parser.getNextData().first->dump(cout);
    }


    return 0;
}

