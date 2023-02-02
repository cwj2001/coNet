//
// Created by 抑~风 on 2023/1/30.
//

#include <iostream>
#include <vector>
#include <string>

#include "util.h"
#include "address.h"

using namespace std;
using namespace CWJ_CO_NET;

int main(){
    cout<<"Hello"<<endl;
    cout<<*(Address::LookupAny("www.baidu.com",AF_INET,SOCK_STREAM))<<endl;
    cout<<*(Address::LookupAny("www.acwing.com",AF_INET,SOCK_STREAM))<<endl;
    return 0;
}

