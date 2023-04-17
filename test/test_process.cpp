//
// Created by 抑~风 on 2023/1/30.
//

#include <iostream>
#include <vector>
#include <string>

//#include "util.h"
#include "process.h"
#include "cwj_process_cycle.h"
using namespace std;
using namespace CWJ_CO_NET;

int main(){

    MasterProcess master(2);
    master.start_process_cycle();
    return 0;
}

