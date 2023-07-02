//
// Created by 抑~风 on 2023/1/30.
//


#include <iostream>
#include <vector>
#include <string>
#include <iomanip>

#include "coroutine.h"
#include "util.h"
#include "log.h"
#include "config.h"
#include "cwj_cpucontext.h"

using namespace std;
using namespace CWJ_CO_NET;

void run(){
    for(int i=0;i<10;i++){
        INFO_LOG(GET_LOGGER("root"))
                                            <<"["<<std::setw(2)
                                            <<std::setfill('0')
                                            <<i<<"]coroutine id : "
                                            <<Coroutine::GetId();
        Coroutine::YieldToReady();
    }
    INFO_LOG(GET_ROOT_LOGGER())<<"run finish";
}

void test(){
    vector<Coroutine::ptr> list;
    for(int i=0;i<10;i++){
        list.emplace_back(new Coroutine(run));
        list.back()->call();
    }
    for(int j=0;j<100;j++) {
        for (int i = 0; i < 10; i++) {
            list[i]->call();
        }
    }
}

int main(){

//    ConfigManager::loadYamlFromDir("/home/cwj2001/cwj/myCppProject/config");
    Thread::SetName("main");
    test();
    cout<<"Hello"<<endl;



    return 0;
}



