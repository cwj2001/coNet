//
// Created by 抑~风 on 2023/1/30.
//

#include <iostream>
#include <vector>
#include <string>

#include "util.h"
#include "fdmanager.h"
#include "socket.h"

using namespace std;
using namespace CWJ_CO_NET;

int main(){

    Socket::ptr server =Socket::CreateTCPSocket();
    server->bind(IPv4Address::Create("0.0.0.0",8033));

    cout<<*FdMgr::GetInstance()->get(server->getSocket(),true)<<endl;

    cout<<"=================="<<endl;





    return 0;
}

