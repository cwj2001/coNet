//
// Created by 抑~风 on 2023-04-15.
//

#ifndef CWJ_CO_NET_CWJ_CHANNEL_H
#define CWJ_CO_NET_CWJ_CHANNEL_H

#include <sys/socket.h>
#include <memory>
namespace CWJ_CO_NET{

struct Channel{
public:
    using ptr = std::shared_ptr<Channel>;
    enum Command{
        CMD_OPEN_CHANNEL,
        CMD_CLOSE_CHANNEL,
        CMD_QUIT,
        CMD_TERMINATE,
        CMD_REOPEN,
    };

    static void WriteChannel(int fd,const Channel& channel);
    static Channel::ptr ReadChannel(int fd);

    Command m_cmd;
    pid_t m_pid;
    u_int32_t m_slot;
    int32_t m_fd;
};

}
#endif //CWJ_CO_NET_CWJ_CHANNEL_H
