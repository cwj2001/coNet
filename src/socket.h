//
// Created by 抑~风 on 2023/2/2.
//

#ifndef CWJ_CO_NET_SOCKET_H
#define CWJ_CO_NET_SOCKET_H

#include <sys/socket.h>

#include <memory>

#include "address.h"

namespace CWJ_CO_NET {
    class Socket {
    public:
        using ptr = std::shared_ptr<Socket>;

        enum Type {
            TCP = SOCK_STREAM,
            UDP = SOCK_DGRAM,
        };

        enum Family {
            IPv4 = AF_INET,
            IPv6 = AF_INET6,
            UNIX = AF_UNIX,
        };

        Socket(int mFamily, int mType, int mProtocol);

        ~Socket();

        bool bind(const Address::ptr addr);

        bool listen(int backlog);

        Socket::ptr accept();

        bool close();

        bool connect(const Address::ptr addr, uint64_t timeout_ms = -1);

        bool reconnect(uint64_t timeout_ms = -1);

        int send(const void *buffer, size_t length, int flags = 0);

        int send(const iovec *buffers, size_t length, int flags = 0);

        int sendTo(const void *buffer, size_t length, const Address::ptr to, int flags = 0);

        int sendTo(const iovec *buffers, size_t length, const Address::ptr to, int flags = 0);

        int recv(void *buffer, size_t length, int flags = 0);

        int recv(iovec *buffers, size_t length, int flags = 0);

        int recvFrom(void *buffer, size_t length, Address::ptr from, int flags = 0);

        int recvFrom(iovec *buffers, size_t length, Address::ptr from, int flags = 0);

        int getMFamily() const;

        int getMType() const;

        int getMProtocol() const;

        bool isConnect() const;

        const Address::ptr getMLocalAddr() ;

        const Address::ptr getMRemoteAddr() ;


        bool isValid() const { return m_sock != -1 ;};

        int getError();

        bool getOption(int level, int optname, void *optval, socklen_t *optlen);

        template<typename T>
        bool getOption(int level, int optname, const T *optval){
            return getOption(level,optname,optval,sizeof(optval));
        }

        bool setOption(int level, int optname, const void *optval, socklen_t optlen);

        template<typename T>
        bool setOption(int level, int optname, const T *optval){
            return setOption(level,optname,optval,sizeof(optval));
        }

        const std::string toString() const ;

    private:

        bool newSocket();

        bool init(int sock,bool is_connect);

    public:

        static Socket::ptr CreateTCP(Address::ptr address);

        static Socket::ptr CreateUDP(Address::ptr address);

        static Socket::ptr CreateTCPSocket();

        static Socket::ptr CreateUDPSocket();

        static Socket::ptr CreateTCPSocket6();

        static Socket::ptr CreateUDPSocket6();

        static Socket::ptr CreateUnixTCPSocket();

        static Socket::ptr CreateUnixUDPSocket();


    private:

        int m_sock = -1;
        int m_family = -1;
        int m_type = -1;
        int m_protocol = 0;

        bool m_is_connect = false;

        Address::ptr m_local_addr = nullptr;
        Address::ptr m_remote_addr = nullptr;


    };

    std::ostream& operator<<(std::ostream& os,const Socket& sock);


}

#endif //CWJ_CO_NET_SOCKET_H
