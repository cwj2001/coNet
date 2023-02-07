//
// Created by 抑~风 on 2023/2/7.
//

#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
using namespace std;

// 注意：对于边沿触发而言，其只有一直在epoll中才有一样，
// 否则如果在触发后，重新删除，在添加该事件，那么其添加时，检测到该事件就会重复触发，因为其不知道这个之前触发过

int main(){

    int ev_fd = eventfd(0,0);

    uint64_t u = 1;
    write(ev_fd,&u,sizeof (uint64_t));
    write(ev_fd,&u,sizeof (uint64_t));
    write(ev_fd,&u,sizeof (uint64_t));
    write(ev_fd,&u,sizeof (uint64_t));
    write(ev_fd,&u,sizeof (uint64_t));
    write(ev_fd,&u,sizeof (uint64_t));

    int epoll_fd = epoll_create(1024);

    epoll_event event;
    event.data.fd = ev_fd;
    event.events = EPOLLIN | EPOLLET;

    epoll_ctl(epoll_fd,EPOLL_CTL_ADD,ev_fd,&event);

    epoll_event evs[10];
    int size=0,count =0;
    while((size=epoll_wait(epoll_fd,evs,10,3000))){

        for(int i=0;i<size;i++){
            int fd = evs[i].data.fd;
            if(fd == ev_fd ){
                cout<<"count="<<count;
                if(!count){
                    count++;
                    epoll_ctl(epoll_fd,EPOLL_CTL_DEL,ev_fd,&event);
                    epoll_ctl(epoll_fd,EPOLL_CTL_ADD,ev_fd,&event);
                    continue;
                }
                read(ev_fd,&u,sizeof (uint64_t));
                cout<<u<<endl;
            }
        }
    }



}

