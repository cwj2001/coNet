//
// Created by 抑~风 on 2023/1/30.
//

#include <iostream>
#include <vector>
#include <string>
#include <memory>

#include "timer.h"
#include "iomanager.h"
#include "log.h"
using namespace std;
using namespace CWJ_CO_NET;

static auto g_logger = GET_ROOT_LOGGER();

void run(){
    IOManager::GetThis()->addTimer(2000,[](){
        INFO_LOG(g_logger) << " test================================= ";
    },true);
    INFO_LOG(g_logger)<<"--end--";
}

int main(){

    IOManager::ptr ioManager = std::make_shared<IOManager>(1,true,"ttt");

    ioManager->schedule(run,-1);

    ioManager->start();

    return 0;
}


