//
// Created by 抑~风 on 2023/1/30.
//

#include <iostream>
#include <vector>
#include <string>
#include <sys/eventfd.h>
#include "iomanager.h"
#include "socket.h"
#include "macro.h"
#include "process.h"
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

int fds[2];

void test(){


    CWJ_ASSERT(socketpair(AF_UNIX,SOCK_STREAM,0,fds)!=-1);

    SetNonblock(fds[0]);
    SetNonblock(fds[1]);

    while(true){
        int fd =fds[0];
        auto msg = Channel::ReadChannel(fd);
        INFO_LOG(g_logger) << "msg recv"<<endl;
        CWJ_ASSERT(msg);
        if(!msg){
            break;
        }
    }
}

void common_test(){
    shared_ptr<IOManager> ioManager(new IOManager(1,true,"ttt"));
    ioManager->schedule(accept_handle,-1);
    ioManager->start();
    CWJ_ASSERT(false);
}



int main(){



    int fd = eventfd(0,0);

    SetNonblock(fd);
    cout<<"";
    auto m_master_cycle = [fd] (IOManager::ptr iom) {
        int ind = 0;
        while(ind < 0x3f3f3f3f){
            INFO_LOG(g_logger)<<" m_master_cycle run";
            int t = sleep(1);
            uint64_t buf = 8;
            write(fd,&buf,sizeof(buf));
//            INFO_LOG(g_logger) << "sleep fail :" << t ;
            ind ++;
        }
    };
    auto m_worker_cycle = [fd] (IOManager::ptr iom){
        int ind = 0;
        while(ind < 0x3f3f3f3f){
            INFO_LOG(g_logger)<<" m_worker_cycle run pid:"<<Process::GetProcessId();
            uint64_t buf;
            read(fd,&buf,sizeof(uint64_t));
            INFO_LOG(g_logger) << "====";
            read(fd,&buf,sizeof(uint64_t));
            INFO_LOG(g_logger) << "====";
            read(fd,&buf,sizeof(uint64_t));
            INFO_LOG(g_logger) << "=== eventfd buf read = "<<buf << " === ";
            ind ++;
        }
    };


    int pid =  fork();
//
    shared_ptr<IOManager> ioManager(new IOManager(1,true,"ttt"));

    Process::UpdateProcessId();
    ioManager->schedule(test,-1);
//q
//    Process::UpdateProcessId();
//    INFO_LOG(g_logger) << "after fork" ;
//
    switch (pid) {
        case -1 :
            CWJ_ASSERT(false);
            break;
        case 0:
            ioManager->schedule(std::bind(m_worker_cycle,ioManager),-1);
            ioManager->start();
            ioManager->stop();
            break;
        default:
//            while(true){
//                INFO_LOG(g_logger) << "fa run";
//                sleep(1);
//                if(pid > 0x3f3f3f3f) break;
//            }

            ioManager->schedule(std::bind(m_master_cycle,ioManager),-1);
            ioManager->start();
            ioManager->stop();
            break;
    }


//


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

