//
// Created by 抑~风 on 2023/2/2.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <cstring>
#include <cstdint>
#include <sstream>

// offsetof
#include <cstddef>

#include "address.h"
#include "log.h"

namespace CWJ_CO_NET{

    static auto g_logger = GET_LOGGER("system");


    bool Address::operator<(const Address& rhs) const {
        socklen_t minlen = std::min(getAddrLen(), rhs.getAddrLen());
        int result = memcmp(getAddr(), rhs.getAddr(), minlen);
        if(result < 0) {
            return true;
        } else if(result > 0) {
            return false;
        } else if(getAddrLen() < rhs.getAddrLen()) {
            return true;
        }
        return false;
    }

    bool Address::operator==(const Address& rhs) const {
        return getAddrLen() == rhs.getAddrLen()
               && memcmp(getAddr(), rhs.getAddr(), getAddrLen()) == 0;
    }

    bool Address::operator!=(const Address& rhs) const {
        return !(*this == rhs);
    }


    Address::ptr Address::Create(const sockaddr *addr, socklen_t len) {
        Address::ptr res ;
        switch (addr->sa_family) {
            case AF_INET:
                res.reset(new IPv4Address(*(const sockaddr_in*)addr));
                break;
            case AF_INET6:
                res.reset(new IPv6Address(*(const sockaddr_in6*)addr));
                break;
            default:
                res.reset(new UnknownAddress(*addr));
        }
        return res;
    }

    bool
    Address::Lookup(std::vector<Address::ptr> &result, const std::string &host, int family, int type, int protocol) {

        addrinfo hints,*res;
        memset(&hints,0,sizeof hints);
        hints.ai_family = family;
        hints.ai_socktype = type;
        hints.ai_protocol = protocol;

        int error = getaddrinfo(host.c_str(), nullptr,&hints,&res);
        if(error) {
            ERROR_LOG(g_logger) << "IPAddress::Create(" << host
                                    <<  ") error=" << error
                                << " errno=" << errno << " errstr=" << strerror(errno);
            return false;
        }

        static auto Deleter = [](auto a){ freeaddrinfo(a); };

        std::unique_ptr<addrinfo,decltype(Deleter)> res_ptr(res,Deleter);

        addrinfo * next = res_ptr.get();
        while(next) {
            result.push_back(Create(next->ai_addr, (socklen_t)next->ai_addrlen));
            INFO_LOG(g_logger) << *result.back() << " get  success ";
            next = next->ai_next;
        }

        return true;

    }

    Address::ptr Address::LookupAny(const std::string &host, int family, int type, int protocol) {
        std::vector<Address::ptr> result;
        if(Lookup(result, host, family, type, protocol)) {
            return result[0];
        }
        return nullptr;
    }

    std::shared_ptr<IPAddress>
    Address::LookupAnyIPAddress(const std::string &host, int family, int type, int protocol) {
        std::vector<Address::ptr> result;
        if(Lookup(result, host, family, type, protocol)) {
            for(auto& i : result) {
                IPAddress::ptr v = std::dynamic_pointer_cast<IPAddress>(i);
                if(v) {
                    return v;
                }
            }
        }
        return nullptr;
    }


    IPAddress::ptr IPAddress::Create(const char *host, uint16_t port) {

        addrinfo hints,*res;
        memset(&hints,0,sizeof hints);

        hints.ai_family = AF_UNSPEC;
        hints.ai_flags = AI_NUMERICHOST;

        int error = getaddrinfo(host, nullptr,&hints,&res);
        if(error) {
            ERROR_LOG(g_logger) << "IPAddress::Create(" << host
                                      << ", " << port << ") error=" << error
                                      << " errno=" << errno << " errstr=" << strerror(errno);
            return nullptr;
        }

        static auto Deleter = [](auto a){ freeaddrinfo(a); };

        std::unique_ptr<addrinfo,decltype(Deleter)> res_ptr(res,Deleter);

        IPAddress::ptr result = nullptr;
        addrinfo * next = res_ptr.get();
        try {
            while (next) {
                result = std::dynamic_pointer_cast<IPAddress>(Address::Create(next->ai_addr
                                                                ,(socklen_t) next->ai_addrlen));

                if (result) {
                    result->setPort(port);
                    break;
                }
                next = next->ai_next;
            };
        }catch (std::exception& e){
            ERROR_LOG(g_logger) << "IPAddress::Create(" << host
                                << ", " << port << ") error";
        }
        return nullptr;
    }

    IPv4Address::IPv4Address(uint32_t ip, uint16_t port) {
        memset(&m_addr,0,sizeof(m_addr));
        m_addr.sin_port = htons(port);
        m_addr.sin_family = AF_INET;
        m_addr.sin_addr.s_addr = ip;
    }

    IPv4Address::IPv4Address(const sockaddr_in &addr) {
        m_addr = addr;
    }

    IPv4Address::ptr IPv4Address::Create(const char *addr, uint16_t port) {
        IPv4Address::ptr res(new IPv4Address);
        if(inet_aton(addr,&res->m_addr.sin_addr)<=0){
            ERROR_LOG(g_logger) << "IPv4Address::Create("
                                <<addr<<" , "<<port<<") error"
                                << " errno=" << errno
                                << " errstr=" << strerror(errno);
            return nullptr;
        }
        res->m_addr.sin_family = AF_INET;
        res->m_addr.sin_port = htons(port);
        return res;
    }

    IPAddress::ptr IPv4Address::broadcastAddress(uint32_t prefix_len) {
        return CWJ_CO_NET::IPAddress::ptr();
    }

    IPAddress::ptr IPv4Address::networdAddress(uint32_t prefix_len) {
        return CWJ_CO_NET::IPAddress::ptr();
    }

    IPAddress::ptr IPv4Address::subnetMask(uint32_t prefix_len) {
        return CWJ_CO_NET::IPAddress::ptr();
    }

    uint32_t IPv4Address::getPort() const {
        return ntohs(m_addr.sin_port);
    }

    void IPv4Address::setPort(uint16_t v) {
        m_addr.sin_port = htons(v);
    }

    const sockaddr *IPv4Address::getAddr() const {
        return (sockaddr*) &m_addr;
    }

    sockaddr *IPv4Address::getAddr() {
        return (sockaddr*) &m_addr;
    }

    socklen_t IPv4Address::getAddrLen() const {
        return sizeof (m_addr);
    }

    std::string IPv4Address::toString() const {
        static char dist[INET_ADDRSTRLEN];
        std::stringstream ss;
        inet_ntop(AF_INET,&m_addr.sin_addr.s_addr,dist,sizeof (dist));

        ss << "ipv4["<<"addr: "<<dist<<" port: "<<ntohs(m_addr.sin_port)<<"]";
        return ss.str();
    }


    IPv6Address::IPv6Address(uint16_t *ip, uint16_t port) {
        memset(&m_addr, 0, sizeof(m_addr));
        m_addr.sin6_family = AF_INET6;
        m_addr.sin6_port = htons(port);
        memcpy(&m_addr.sin6_addr.s6_addr, ip, 8);
    }

    IPv6Address::IPv6Address(const sockaddr_in6 &addr) {
        m_addr = addr;
    }

    IPv6Address::ptr IPv6Address::Create(const char *addr, uint16_t port) {
        IPv6Address::ptr res(new IPv6Address);
        if(inet_pton(AF_INET6,addr,&res->m_addr.sin6_addr)<=0){
            ERROR_LOG(g_logger) << "IPv6Address::Create("
                                <<addr<<" , "<<port<<") error"
                                << " errno=" << errno
                                << " errstr=" << strerror(errno);
            return nullptr;
        }
        res->m_addr.sin6_family = AF_INET;
        res->m_addr.sin6_port = htons(port);
        return res;
    }

    const sockaddr *IPv6Address::getAddr() const {
        return (sockaddr*) &m_addr;
    }

    sockaddr *IPv6Address::getAddr() {
        return (sockaddr*) &m_addr;
    }

    socklen_t IPv6Address::getAddrLen() const {
        return sizeof (m_addr);
    }
    std::string IPv6Address::toString() const {
        static char dist[INET6_ADDRSTRLEN];
        memset(dist,0,sizeof dist);
        inet_ntop(AF_INET6,&m_addr.sin6_addr,dist,(socklen_t)sizeof(dist));
        std::stringstream ss;
        ss << "ipv6[ "<<" addr :"<<dist<<" port: "<<ntohs(m_addr.sin6_port)<<" ]";
        return ss.str();
    }

    IPAddress::ptr IPv6Address::broadcastAddress(uint32_t prefix_len) {
        return CWJ_CO_NET::IPAddress::ptr();
    }

    IPAddress::ptr IPv6Address::networdAddress(uint32_t prefix_len) {
        return CWJ_CO_NET::IPAddress::ptr();
    }

    IPAddress::ptr IPv6Address::subnetMask(uint32_t prefix_len) {
        return CWJ_CO_NET::IPAddress::ptr();
    }

    uint32_t IPv6Address::getPort() const {
        return ntohs(m_addr.sin6_port);
    }

    void IPv6Address::setPort(uint16_t v) {
        m_addr.sin6_port = htons(v);
    }

    IPv6Address::IPv6Address() {
        memset(&m_addr,0,sizeof m_addr);
        m_addr.sin6_family = AF_INET6;
    }


    UnixAddress::UnixAddress() {
        memset(&m_addr,0,sizeof m_addr);
        m_addr.sun_family = AF_UNIX;
        m_len = sizeof m_addr;
    }

    UnixAddress::UnixAddress(const std::string &path) {
        m_len = path.size() ? path.size()+1 : 0;
        if(m_len > sizeof(m_addr.sun_path)) {
            throw std::logic_error("path too long");
        }
        memset(&m_addr,0,sizeof m_addr);
        m_addr.sun_family = AF_UNIX;
        memcpy(m_addr.sun_path,path.c_str(),path.size());
        m_len += offsetof(sockaddr_un, sun_path);
    }

    const sockaddr *UnixAddress::getAddr() const {
        return (sockaddr*) &m_addr;
    }

    sockaddr *UnixAddress::getAddr() {
        return (sockaddr*) &m_addr;
    }

    socklen_t UnixAddress::getAddrLen() const {
        return sizeof (m_addr);
    }
    std::string UnixAddress::toString() const {
        std::stringstream ss;
        ss << "UnixAddress [ path: "<<m_addr.sun_path<<" ] ";
        return ss.str();
    }

    UnixAddress::ptr UnixAddress::Create(const std::string &path) {
        return std::make_shared<UnixAddress>(path);
    }

    UnknownAddress::UnknownAddress(int family) {
        memset(&m_addr,0,sizeof m_addr);
        m_addr.sa_family = family;
    }

    UnknownAddress::UnknownAddress(const sockaddr &addr) {
        m_addr = addr;
    }

    const sockaddr *UnknownAddress::getAddr() const {
        return &m_addr;
    }

    sockaddr *UnknownAddress::getAddr() {
        return &m_addr;
    }

    socklen_t UnknownAddress::getAddrLen() const {
        return sizeof m_addr;
    }

    std::string UnknownAddress::toString() const {
        return  "[UnknownAddress family=" + std::to_string(m_addr.sa_family) + "]";;
    }


    std::ostream &operator<<(std::ostream &os, const Address &addr) {
        os << addr.toString();
        return os;
    }


}
