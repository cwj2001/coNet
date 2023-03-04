//
// Created by 抑~风 on 2023/1/30.
//

#include <signal.h>
#include <iostream>
#include <vector>
#include <string>
#include "http/http_server.h"
#include "util.h"
#include "socket.h"
#include "config.h"

//#include "newoperator.h"

using namespace std;
using namespace CWJ_CO_NET;
using namespace CWJ_CO_NET::http;
int main(){

//    signal (SIGPIPE, SIG_IGN);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &sa, NULL);

    ConfigManager::loadYamlFromDir("/home/cwj2001/cwj/myCppProject/config");

    ServletDispatch dispatch;
    dispatch.addServlet("/name",[](auto req, auto resp){});
    dispatch.addGlobServlet("/name/cwj/*",[](HttpRequest::ptr req, HttpResponse::ptr resp){
        resp->setHeader("cwj","2001");
        resp->setHeader("path",req->getPath());
        resp->setBody("cwj2001 body ,path="+req->getPath());
    });

    TcpServer::ptr server(new HttpServer("http_server",1,2,false,dispatch));

    if(!server->bind(IPv4Address::Create("0.0.0.0", 8033))) {
        CWJ_ASSERT(false);
    };
    server->start();

    return 0;
}

