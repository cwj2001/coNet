//
// Created by 抑~风 on 2023/1/30.
//

#include <iostream>
#include <vector>
#include <string>

#include "iomanager.h"
#include "socket.h"
#include "macro.h"

using namespace std;
using namespace CWJ_CO_NET;

static auto g_logger = GET_ROOT_LOGGER();

void client_handle(Socket::ptr &client);

void accept_handle(){
    Socket::ptr server = Socket::CreateTCPSocket();
    if(server->bind(IPv4Address::Create("0.0.0.0",8033)) &&  server->listen(1024)){
        while(true) {
            IOManager::GetThis()->addEvent(server->getSocket(), IOManager::READ);
            Coroutine::YieldToHold();
            Socket::ptr client = server->accept();
            INFO_LOG(g_logger) <<"accept : "<< *client;
            if(client<0) break;
            client->setNonBlock();
            IOManager::GetThis()->addEvent(client->getSocket(), IOManager::READ, bind(client_handle, client));
        }
    }

}

int main(){
   shared_ptr<IOManager> ioManager(new IOManager(1,true,"ttt"));
   ioManager->schedule(accept_handle,-1);
   ioManager->start();
    CWJ_ASSERT(false);
    return 0;
}

void client_handle(Socket::ptr &client) {
       static char buf[10];
       memset(buf,0,sizeof(buf));
       string str;
       while(client->isConnect()){
           str.clear();
           int rt = 0;
           while((rt=client->recv(buf,10,0))>0){
               str = buf;
               str = "server :" + str;
               if(strlen(buf) == 0) continue;
               INFO_LOG(g_logger)<<"client: "<<*client<<" send:"<<str;
               client->send(str.c_str(),str.size(),0);
               memset(buf,0,sizeof(buf));
           }
            if(rt == -1 && ( errno != EAGAIN && errno != EWOULDBLOCK)){
                ERROR_LOG(g_logger) << "client error ,errno="<<errno<<" strerror= "<<strerror(errno);
            }else if(rt == 0){
                ERROR_LOG(g_logger) << "client"<<*client<<" close"<<"";
                client->close();
                break;
            }
           IOManager::GetThis()->addEvent(client->getSocket(),IOManager::READ);
           Coroutine::YieldToHold();

       }
       str = "bye";
       client->send(str.c_str(),str.size(),0);
}

