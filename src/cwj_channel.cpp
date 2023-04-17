//
// Created by 抑~风 on 2023-04-15.
//
#include "cwj_channel.h"
#include "macro.h"
#include "log.h"
#include <unistd.h>
#include <cstring>

namespace CWJ_CO_NET{
    static auto g_logger = GET_LOGGER("system");
    void Channel::WriteChannel(int fd,const Channel& channel) {
        int size = sizeof(Channel),len = size;
        auto buf = (char *)&channel;
        int cnt = 0;
        while(cnt < size){
            int t = 0;
            if((t = send(fd,buf+cnt,size-cnt,0)) , t == -1){
                ERROR_LOG(g_logger) << "fd="<<fd<<" recv fail "<<"errno="<<errno<<" strerror= "<<strerror(errno);
                CWJ_ASSERT(false);
            }
            cnt += t;
        }
        return ;
    }

    Channel::ptr Channel::ReadChannel(int fd) {
        int size = sizeof(Channel),len = size;
        auto ans = std::make_shared<Channel>();
        auto buf = (char *)ans.get();
        int cnt = 0;
        while(cnt < size){
            int t = 0;
            if((t = recv(fd,buf+cnt,size-cnt,0)) == -1){
                ERROR_LOG(g_logger) << "fd="<<fd<<" recv fail "<<"errno="<<errno<<" strerror= "<<strerror(errno);
                CWJ_ASSERT(false);
                return ans;
            }
            cnt += t;
        }
        return ans;
    }
}
