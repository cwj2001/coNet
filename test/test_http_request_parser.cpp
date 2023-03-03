//
// Created by 抑~风 on 2023/1/30.
//

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include "http/http_request_parser.h"
#include "macro.h"
//#include "util.h"

using namespace std;
using namespace CWJ_CO_NET::http;
using namespace CWJ_CO_NET;

int main(){
    cout<<"Hello"<<endl;

    const char* buf;
    float start, end;
    size_t parsed;

    auto buf_1 =  "GET http://admin.omsg.cn/uploadpic/2016121034000012.png HTTP/1.1\r\nHost: /"
           "admin.omsg.cn\r\nAccept:*/*\r\nConnection: Keep-Alive\r\n\r\n";

    const char * buf_3 = "GET http://admin.omsg.cn/uploadpic/2016121034000012.png HTTP/1.1\r\nHost: /"
           "admin.omsg.cn\r\nAccept:*/*\r\nConnection: Keep-Alive\r\n\r\n";
    const char * buf_2 = "POST http://mcp/pc/pcsearch HTTP/1.1\r\n"
                         "Accept: */*\r\n"
                         "Accept-Encoding: gzip, deflate, br\n"
                         "Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6\n"
                         "Access-Control-Request-Headers: content-type\n"
                         "Access-Control-Request-Method: POST\n"
                         "Connection: keep-alive\n"
                         "Host: ug.baidu.com\n"
                         "Origin: https://www.baidu.com\n"
                         "Referer: https://www.baidu.com/s?ie=utf-8&f=8&rsv_bp=1&rsv_idx=1&tn=baidu&wd=WEF&fenlei=256&rsv_pq=0xc96e8353000667c5&rsv_t=c048EntTZTwNzOR9XKx%2BQA6zYga%2B%2BlbQP6s55I47750zSFd4dltNOoe9JtoP&rqlang=en&rsv_enter=1&rsv_dl=tb&rsv_sug2=0&rsv_btype=i&inputT=666&rsv_sug4=766\n"
                         "Sec-Fetch-Dest: empty\n"
                         "Sec-Fetch-Mode: cors\n"
                         "Sec-Fetch-Site: same-site\n"
                         "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/110.0.0.0 Safari/537.36 Edg/110.0.1587.57\r\n\r\n";

    const char * buf_4 = "POST http://mcp/pc/pcsearch HTTP/1.1\r\n"
                         "Accept: */*\r\n"
                         "Accept-Encoding: gzip, deflate, br\n"
                         "Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6\n"
                         "Access-Control-Request-Headers: content-type\n"
                         "Access-Control-Request-Method: POST\n"
                         "Connection: keep-alive\n"
                         "Host: ug.baidu.com\n"
                         "Origin: https://www.baidu.com\n"
                         "Referer: https://www.baidu.com/s?ie=utf-8&f=8&rsv_bp=1&rsv_idx=1&tn=baidu&wd=WEF&fenlei=256&rsv_pq=0xc96e8353000667c5&rsv_t=c048EntTZTwNzOR9XKx%2BQA6zYga%2B%2BlbQP6s55I47750zSFd4dltNOoe9JtoP&rqlang=en&rsv_enter=1&rsv_dl=tb&rsv_sug2=0&rsv_btype=i&inputT=666&rsv_sug4=766\n"
                         "Sec-Fetch-Dest: empty\n"
                         "Sec-Fetch-Mode: cors\n"
                         "Sec-Fetch-Site: same-site\n"
                         "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/110.0.0.0 Safari/537.36 Edg/110.0.1587.57\r\n\r\n"
                         "GET http://admin.omsg.cn/uploadpic/2016121034000012.png HTTP/1.1\r\nHost: /"
                         "admin.omsg.cn\r\nAccept:*/*\r\nConnection: Keep-Alive\r\n\r\n";
                         ;


    HttpRequestParser parser;

    cout<<"1.----------------------"<<endl;
    parser.execute(buf_1,strlen(buf_1));
    parser.getNextData().first->dump(cout);

    cout<<"2.----------------------"<<endl;
    parser.execute(buf_2,strlen(buf_2));
    parser.getNextData().first->dump(cout);

    cout<<"3.----------------------"<<endl;
    parser.execute(buf_3,strlen(buf_3));
    parser.getNextData().first->dump(cout);

    cout<<"4.-------------------------"<<endl;

    parser.execute(buf_4,strlen(buf_4));

    while(parser.hasNext()){
        parser.getNextData().first->dump(cout);
    }

    return 0;
}

