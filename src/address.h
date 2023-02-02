//
// Created by 抑~风 on 2023/2/2.
//

#ifndef CWJ_CO_NET_ADDRESS_H
#define CWJ_CO_NET_ADDRESS_H

#include <memory>
#include <vector>

// 下面三个头文件：sockaddr sockaddr_in sockaddr_in6 socklen_t
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h> // sockaddr_un


namespace CWJ_CO_NET {

    class IPAddress;

    class Address {
    public:
        using ptr = std::shared_ptr<Address>;

        virtual ~Address() {};

        int getFamily() const { return getAddr()->sa_family; };

        virtual const sockaddr *getAddr() const = 0;

        virtual sockaddr *getAddr() = 0;

        virtual socklen_t getAddrLen() const = 0;

        virtual std::string toString() const = 0;

        bool operator<(const Address &rhs) const;

        bool operator==(const Address &rhs) const;

        bool operator!=(const Address &rhs) const;


    public:
        static ptr Create(const sockaddr *addr, socklen_t len);

        static bool
        Lookup(std::vector<Address::ptr> &result, const std::string &host, int family = AF_INET, int type = 0, int protocol = 0);

        static Address::ptr LookupAny(const std::string &host, int family = AF_INET, int type = 0, int protocol = 0);

        static std::shared_ptr<IPAddress> LookupAnyIPAddress(const std::string &host,
                                                             int family = AF_INET, int type = 0, int protocol = 0);


    private:
    };

    class IPAddress : public Address {
    public:
        using ptr = std::shared_ptr<IPAddress>;

        static IPAddress::ptr Create(const char *host, uint16_t port);

        virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;

        virtual IPAddress::ptr networdAddress(uint32_t prefix_len) = 0;

        virtual IPAddress::ptr subnetMask(uint32_t prefix_len) = 0;

        virtual uint32_t getPort() const = 0;

        virtual void setPort(uint16_t v) = 0;

    };

    class IPv4Address : public IPAddress {
    public:
        using ptr = std::shared_ptr<IPv4Address>;

        IPv4Address(uint32_t ip = INADDR_ANY, uint16_t port = 0);

        IPv4Address(const sockaddr_in &addr);

        const sockaddr *getAddr() const override;

        sockaddr *getAddr() override;

        socklen_t getAddrLen() const override;

        std::string toString() const override;

        IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;

        IPAddress::ptr networdAddress(uint32_t prefix_len) override;

        IPAddress::ptr subnetMask(uint32_t prefix_len) override;

        uint32_t getPort() const;

        void setPort(uint16_t v);

    public:
        static IPv4Address::ptr Create(const char *addr, uint16_t port);

    private:
        sockaddr_in m_addr;
    };

    class IPv6Address : public IPAddress {
    public:
        using ptr = std::shared_ptr<IPv6Address>;

        IPv6Address();

        IPv6Address(uint16_t ip[8], uint16_t port = 0);

        IPv6Address(const sockaddr_in6 &addr);

        const sockaddr *getAddr() const override;

        sockaddr *getAddr() override;

        socklen_t getAddrLen() const override;

        std::string toString() const override;

        IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;

        IPAddress::ptr networdAddress(uint32_t prefix_len) override;

        IPAddress::ptr subnetMask(uint32_t prefix_len) override;

        uint32_t getPort() const;

        void setPort(uint16_t v);

    public:
        static IPv6Address::ptr Create(const char *addr, uint16_t port);

    private:
        sockaddr_in6 m_addr;
    };

    class UnixAddress : public Address{
    public:
        using ptr = std::shared_ptr<Address>;

        UnixAddress();

        UnixAddress(const std::string& path);

        const sockaddr *getAddr() const override;

        sockaddr *getAddr() override;

        socklen_t getAddrLen() const override;

        std::string toString() const override;

    public:

        static UnixAddress::ptr Create(const std::string& path);

    private:
        sockaddr_un m_addr;
        socklen_t m_len;
    };


    class UnknownAddress : public Address {
    public:
        typedef std::shared_ptr<UnknownAddress> ptr;
        UnknownAddress(int family);
        UnknownAddress(const sockaddr& addr);
        const sockaddr* getAddr() const override;
        sockaddr* getAddr() override;
        socklen_t getAddrLen() const override;
        std::string toString() const override;
    private:
        sockaddr m_addr;
    };

    std::ostream& operator<<(std::ostream& os, const Address& addr);

}

#endif //CWJ_CO_NET_ADDRESS_H
