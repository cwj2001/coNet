//
// Created by 抑~风 on 2023/1/30.
//

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include "http/http_session.h"
#include "http/http_request_parser.h"
#include "util.h"
#include "address.h"
#include "socket.h"
#include "macro.h"

using namespace std;
using namespace CWJ_CO_NET;
using namespace CWJ_CO_NET::http;
int main(){
    const char * buf = "OPTIONS /mcp/pc/pcsearch HTTP/1.1\n"
                "Accept: */*\n"
                "Accept-Encoding: gzip, deflate, br\n"
                "Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6\n"
                "Access-Control-Request-Headers: content-type\n"
                "Access-Control-Request-Method: POST\n"
                "Connection: keep-alive\n"
                "Host: ug.baidu.com\n"
                "Origin: https://www.baidu.com\n"
                "Referer: https://www.baidu.com/s?ie=utf-8&f=8&rsv_bp=1&rsv_idx=2&tn=baidutop10&wd=%E8%BF%88%E5%90%91%E7%8E%B0%E4%BB%A3%E5%8C%96%E5%BC%BA%E5%9B%BD%E7%9A%84%E5%8F%91%E5%B1%95%E5%AF%86%E7%A0%81&oq=%25E8%25BF%2588%25E5%2590%2591%25E7%258E%25B0%25E4%25BB%25A3%25E5%258C%2596%25E5%25BC%25BA%25E5%259B%25BD%25E7%259A%2584%25E5%258F%2591%25E5%25B1%2595%25E5%25AF%2586%25E7%25A0%2581&rsv_pq=88e1974900049e1c&rsv_t=329aA1f37vRTEeDV543ol5trC7%2BBGLpqS7nvLXLmlgAO32FUcQU5YTwDMRQ63rLRYw&rqlang=cn&rsv_enter=1&rsv_dl=tb&rsv_sug2=0&rsv_btype=t&inputT=4&rsv_sug4=4\n"
                "Sec-Fetch-Dest: empty\n"
                "Sec-Fetch-Mode: cors\n"
                "Sec-Fetch-Site: same-site\n"
                "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/110.0.0.0 Safari/537.36 Edg/110.0.1587.57"
                ;

    HttpRequestParser parser;

    auto req = parser.execute(buf,strlen(buf));

    auto addr = IPv4Address::Create("www.baidu.com",80);
    auto sock = Socket::CreateTCP(addr);
    CWJ_ASSERT(!sock->connect(addr));
    HttpSession session(sock);

    return 0;
}

