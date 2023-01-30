#include <iostream>
#include <fstream>
#include <vector>
#include <pthread.h>
#include "util.h"
#include "mutex.h"
using namespace std;
using namespace CWJ_CO_NET;

static volatile size_t num = 0;
static Mutex g_mutex;
static CASLock g_cas;
static RWMutex g_rw_mutex;
void* add(void * a){
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
    return 0;
}

void* addNum(void *){
    for(int i=0;i<50000;i++){
//        CASLock::Lock lock(g_cas);
RWMutex::WLock lock(g_rw_mutex);
        num++;
    }
    return 0;
};

int main() {
    vector<pthread_t>list(10);
    for(int i=0;i<10;i++){
        pthread_create(&list[i], nullptr,(void*(*)(void *))(&addNum),&list);
    }

    for(int i=0;i<10;i++){
        pthread_join(list[i],nullptr);
    }

    cout<<num<<endl;

    return 0;

    return 0;
}
