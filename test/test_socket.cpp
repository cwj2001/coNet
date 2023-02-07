//
// Created by 抑~风 on 2023/1/30.
//

#include <iostream>
#include <vector>
#include <string>

#include "socket.h"
#include "address.h"
#include "util.h"

void test_client();

void test_server();

using namespace std;
using namespace CWJ_CO_NET;

int main(){

    auto addr = IPv4Address::Create("192.168.23.134", 8034);
    auto sock = Socket::CreateTCPSocket();
    while(sock->connect(addr)) {

    }

    return 0;
}

void test_server() {
    Socket::ptr sock = Socket::CreateTCPSocket();

    Address::ptr addr = IPv4Address::Create("0.0.0.0",8033);

    char buf[10];
    if(sock->bind(addr) && sock->listen(1024)){
        cout<<*sock->getMLocalAddr()<<endl;
        Socket::ptr client = sock->accept();
        while(client->isConnect() && client->recv(buf, sizeof(buf),0)>0){
            string str(buf);
            str = "server:"+str;
            client->send(str.c_str(),str.size(),0);
            memset(buf,0,sizeof (buf));
        }
        cout<<"connection :"<<*client<<" close "<<endl;
    }
}

void test_client() {
    Address::ptr addr = Address::LookupAny("www.baidu.com", AF_INET, SOCK_STREAM, 0);

    auto ptr = dynamic_pointer_cast<IPAddress>(addr);
    ptr->setPort(((uint16_t)80));

    cout<<*addr<<endl;

    string buf = "GET / HTTP/1.1\r\n"
                       "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n"
                       "Accept-Encoding: gzip, deflate, br\r\n"
                       "Accept-Language: zh-CN,zh;q=0.9,en;q=0.8,en-GB;q=0.7,en-US;q=0.6\r\n"
                       "Cache-Control: max-age=0\r\n"
                       "Connection: keep-alive\r\n\r\n";

    Socket::ptr sock = Socket::CreateTCP(addr);
    static char recv_buf[1024*1024] = {0};
    if(sock->connect(addr,-1)){
        cout<<"==============="<<endl;
        sock->send((void *)buf.c_str(),buf.size(),0);
        cout<<"==============="<<endl;
        while(sock->recv((void*)recv_buf,128*1024,0)>0){
            cout<<recv_buf<<endl;
            memset(recv_buf,0,128*1024);
        }

    }
}

