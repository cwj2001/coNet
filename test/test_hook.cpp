//
// Created by 抑~风 on 2023/1/30.
//

#include <iostream>
#include <vector>
#include <string>

#include "util.h"
#include "hook.h"
#include "iomanager.h"
#include "macro.h"
#include "socket.h"
#include "log.h"
using namespace std;
using namespace CWJ_CO_NET;

static auto g_logger = GET_ROOT_LOGGER();

void run(){
    SetHookEnable(true);
    auto server = Socket::CreateTCPSocket();
    if(server && server->bind(IPv4Address::Create("0.0.0.0",8033)) && server->listen(1024)) {
        while (true) {
            auto client = server->accept();
            if(!client) {
                INFO_LOG(g_logger) << "quit server";
                break;
            }
            IOManager::GetThis()->schedule([client](){
                ERROR_LOG(GET_ROOT_LOGGER()) << "client:"<<*client<<" connect ";
                static char buf[10];
                while(client && client->isConnect() && client->recv(buf,sizeof(buf),0)>0){
                    if(strcmp(buf,"\r\n")) {
                        string str;
                        str = buf;
                        str = "server:" + str;
                        client->send(str.c_str(), str.size(), 0);
                    }
                    memset(buf,0,sizeof(buf));
                }
                ERROR_LOG(GET_ROOT_LOGGER()) << "client:"<<*client<<" close ";
            },-1);
        }
    }

    INFO_LOG(g_logger)<<"quit run";
    IOManager::GetThis()->auto_stop();

}

int main(){

    IOManager::ptr ioManager(new IOManager(1,true,"tttt"));
    ioManager->schedule(run,-1);
    ioManager->start();
    ioManager->stop();
    return 0;
}

