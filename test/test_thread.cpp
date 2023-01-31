//
// Created by 抑~风 on 2023/1/30.
//
#include <iostream>
#include <fstream>
#include <vector>
#include <pthread.h>
#include "util.h"
#include "mutex.h"
#include "thread.h"
#include "macro.h"
using namespace std;
using namespace CWJ_CO_NET;

static volatile size_t num = 0;
static Mutex g_mutex;
static CASLock g_cas;
static RWMutex g_rw_mutex;

void run_and_test_Thread();

void add(){
//    cout<<"thread"<<GetThreadId()<<"run add"<<endl;
    while(true){
        {
            Mutex::Lock mx(g_mutex);
            if (num >= 100000000) {
                break;
            }

            num++;
        }
    }
    cout<<num<<endl;
}

void addNum(){
    for(int i=0;i<50000;i++){
//        CASLock::Lock lock(g_cas);
        RWMutex::WLock lock(g_rw_mutex);
        num++;
    }
//    sleep(100);
//    cout<<endl;
};

int main() {

    cout<<Thread::GetName()<<endl;
    run_and_test_Thread();


//    sleep(1000000);

    return 0;

    return 0;
}

void run_and_test_Thread() {
    for(int j=0; j < 100; j++){
        num = 0;
        vector<Thread::ptr>list;
        for(int i=0;i<10;i++){
            list.push_back(Thread::ptr(new Thread("thread_"+to_string(i),addNum)));
            list.back()->start();
        }

        for(auto a :list){
            a->join();
        }
        cout<<500000<<" "<<num<<(num == 500000)<<endl;
        CWJ_ASSERT(num == 500000);
        cout<<"-----------------------------------"<<endl<<endl;

    }
}


