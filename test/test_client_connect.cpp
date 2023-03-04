//
// Created by 抑~风 on 2023/1/30.
//

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <fstream>

#include "socket.h"
#include "iomanager.h"
#include "log.h"
#include "hook.h"
#include "config.h"
//#include "util.h"

using namespace std;
using namespace CWJ_CO_NET;

static auto g_logger = GET_ROOT_LOGGER();

static vector<string> test_data;
static string filename = "./other/test_data.txt";

void test_time_recv_send(size_t peer_send_size, const vector<string> &test, vector<string> &recv_list, int& ind,
                         vector<Socket::ptr> &list, int &count);

void do_connect(const char *ip, int port) {

    CWJ_ASSERT(test_data.size());

    size_t connect_size = 100,peer_send_size = 1000;
    vector<string>test;
    vector<string>recv_list;
    test.reserve(connect_size*peer_send_size);
    recv_list.reserve(connect_size*peer_send_size);
    cout << ip << " " << port << endl;

    vector<Socket::ptr> list;

    for (int i = 0; i < connect_size; i++) {
        int new_port = port;
        auto addr = IPv4Address::Create(ip, new_port);
        auto sock = Socket::CreateTCPSocket();
        CWJ_ASSERT(sock->getSocket()<0);
        if (sock->connect(addr)) {
            list.push_back(sock);
        } else {
            cout << "sock connect error " << *sock << endl;
        }
    }


    INFO_LOG(g_logger) << list.size();
    for(int i=0;i<peer_send_size*connect_size;i++){
        test.emplace_back(test_data[rand() % test_data.size()]);
    }
    int success_count = 0,count=0,ind=0;

    CWJ_ASSERT(recv_list.size() == 0 && test.size() == peer_send_size*connect_size);

    auto time = CalcTime(&test_time_recv_send,peer_send_size, test, recv_list, ind, list, count);


    INFO_LOG(g_logger)<<"\n"
            <<"connect_size="<<connect_size
            <<" peer_send_size="<<peer_send_size
            <<" count="<<count
            << " success_count= "<<success_count
            <<" sock_size:"<<list.size()
            <<" time="<<time<<endl;
    IOManager::GetThis()->auto_stop();
}

void test_time_recv_send(size_t peer_send_size, const vector<string> &test, vector<string> &recv_list, int& ind,
                         vector<Socket::ptr> &list, int &count) {
    unique_ptr<char[]> ptr(new char[1024]);
    for (int i = 0; i < peer_send_size; i++) {
        for (int j=0;j<list.size();j++) {
            auto& a = list[j];
            int recv_size=0,send_size=0;
            memset(ptr.get(), 0, sizeof(1024));

            const auto &t = test[ind];
            if ((send_size=a->send(t.c_str(), t.size())) < 0 || (recv_size=a->recv(ptr.get(), 1024)) < 0) {
                if(( errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR))   continue;
                if(send_size<=0 )    cout <<"send:"<< *a << "error,errno="<<errno<<" strerror= "<<strerror(errno) << endl;
                else cout <<"recv:" << *a << "error,errno="<<errno<<" strerror= "<<strerror(errno) << endl;
                continue;
            }
            recv_list.emplace_back(ptr.get(),recv_size);
//            cout<<string(ptr.get(),recv_size)<<endl;
            count++;
            ind++;

            // 240000 250000
// 242500 245000
// 241500 242500
// 240000 247500
            if(count>240000){
                cout<<count<<endl;
            }
            if(count == 240027){
                cout<<"";
            }
//            if(count == 242500){
//                CWJ_ASSERT(false);
//            }

        }
//        cout<<count<<endl;

    }
}

int main(int argc, char **argv) {
    if (argc <= 2) {
        printf("Usage: %s ip port\n", argv[0]);
        exit(0);
    }

    ifstream out(filename);

    while (out) {
        string str;
        getline(out, str);
        test_data.push_back(std::move(str));
    }

//    ConfigManager::loadYamlFromDir("/home/cwj2001/cwj/myCppProject/config");

//    const char *ip = argv[1];
//    int port = atoi(argv[2]);

    const char *ip = "192.168.23.134";
    int port = 8035;

    IOManager::ptr iom(new IOManager(1,true,"client_test"));

    iom->schedule(std::bind(&do_connect,ip,port),-1);
    iom->start();
    iom->stop();

    return 0;
}

