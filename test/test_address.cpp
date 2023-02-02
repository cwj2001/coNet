//
// Created by 抑~风 on 2023/1/30.
//

#include <iostream>
#include <vector>
#include <string>

#include "util.h"
#include "address.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "log.h"
using namespace std;
using namespace CWJ_CO_NET;
static auto g_logger = GET_ROOT_LOGGER();

static bool Lookup(std::vector<Address::ptr>& result, const std::string& host,
                   int family, int type, int protocol) {
    addrinfo hints, *results, *next;
    hints.ai_flags = 0;
    hints.ai_family = family;
    hints.ai_socktype = type;
    hints.ai_protocol = protocol;
    hints.ai_addrlen = 0;
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    std::string node;
    const char* service = NULL;

    //检查 ipv6address serivce
    if(!host.empty() && host[0] == '[') {
        const char* endipv6 = (const char*)memchr(host.c_str() + 1, ']', host.size() - 1);
        if(endipv6) {
            //TODO check out of range
            if(*(endipv6 + 1) == ':') {
                service = endipv6 + 2;
            }
            node = host.substr(1, endipv6 - host.c_str() - 1);
        }
    }

    //检查 node serivce
    if(node.empty()) {
        service = (const char*)memchr(host.c_str(), ':', host.size());
        if(service) {
            if(!memchr(service + 1, ':', host.c_str() + host.size() - service - 1)) {
                node = host.substr(0, service - host.c_str());
                ++service;
            }
        }
    }

    if(node.empty()) {
        node = host;
    }
    int error = getaddrinfo(node.c_str(), service, &hints, &results);
    if(error) {
        DEBUG_LOG(g_logger) << "Address::Lookup getaddress(" << host << ", "
                                  << family << ", " << type << ") err=" << error << " errstr="
                                  << strerror(error);
        return false;
    }

    next = results;
    while(next) {
        result.push_back(Address::Create(next->ai_addr, (socklen_t)next->ai_addrlen));
        //SYLAR_LOG_INFO(g_logger) << ((sockaddr_in*)next->ai_addr)->sin_addr.s_addr;
        next = next->ai_next;
    }

    freeaddrinfo(results);
    return !result.empty();
}

int main(){
    cout<<"Hello"<<endl;
    cout<<*(Address::LookupAny("www.baidu.com",AF_INET,SOCK_STREAM))<<endl;
    cout<<*(Address::LookupAny("www.acwing.com",AF_INET,SOCK_STREAM))<<endl;
    std::vector<Address::ptr> result;
    cout<<"==================="<<endl;
    if(Lookup(result,"www.baidu.com", AF_INET, SOCK_STREAM, 0)) {
        for(auto& a : result){
            cout<<*a<<endl;
        }
    }
    return 0;
}

