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

using namespace std;
using namespace CWJ_CO_NET;
using namespace CWJ_CO_NET::http;
int main(){

    signal (SIGPIPE, SIG_IGN);

    ServletDispatch dispatch;
    dispatch.addServlet("/name",[](auto req, auto resp){});
    dispatch.addGlobServlet("/name/cwj/*",[](HttpRequest::ptr req, HttpResponse::ptr resp){
        resp->setHeader("cwj","2001");
        resp->setHeader("path",req->getPath());
        resp->setBody("cwj2001 body ,path="+req->getPath());
    });

    TcpServer::ptr server(new HttpServer("http_server",1,1,false,dispatch));

    server->bind(IPv4Address::Create("0.0.0.0", 8033));
    server->start();

    return 0;
}

