//
// Created by guo on 2025/7/24.
//

#ifndef SOLAR_ADDRESS_H
#define SOLAR_ADDRESS_H
#include <memory>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/un.h>
namespace solar {
class IPAddress;
class Address {
public:
    typedef std::shared_ptr<Address> ptr;

    static  Address::ptr Create(const sockaddr* addr, socklen_t addrlen);

    /**
     *
     * @param result
     * @param host 域名,服务器名等,形式为 name[:service] (方括号可以没有). 举例: www.google.com[:80]
     * @param family
     * @param type
     * @param protocol
     * @return
     */
    static  bool LookUp(std::vector<Address::ptr>& result, const std::string& host,
                        int family = AF_INET, int type = 0, int protocol = 0);

    static Address::ptr LookUpAny(const std::string& host,
        int family = AF_INET, int type = 0, int protocol = 0);

    static std::shared_ptr<IPAddress> LookUpAnyIpAddress(const std::string &host,
        int family = AF_INET, int type = 0, int protocol = 0);

    static bool GetInterfaceAddresses(std::multimap<std::string, std::pair<Address::ptr, uint32_t>>& result,
        int family = AF_INET);

    static bool GetInterfaceAddresses(std::vector<std::pair<Address::ptr, uint32_t>>& result,
        const std::string& iface, int family = AF_INET);

    virtual ~Address() {}
    int getFamily() const;

    virtual const sockaddr *getAddr() const = 0;
    virtual sockaddr *getAddr() = 0;
    virtual size_t getAddrLen() const = 0;
    virtual std::ostream &insert(std::ostream &os) const = 0;
    std::string toString() const;

    bool operator<(const Address &rhs) const;
    bool operator==(const Address &rhs) const;
    bool operator!=(const Address &rhs) const;
};

class IPAddress : public Address {
public:
    typedef std::shared_ptr<IPAddress> ptr;

    static IPAddress::ptr Create(const char* address, uint16_t port = 0);

    ~IPAddress() override {}

    virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len) = 0;
    virtual IPAddress::ptr networkAddress(uint32_t prefix_len) = 0;
    virtual IPAddress::ptr subnetMask(uint32_t prefix_len) = 0;

    virtual uint16_t getPort() const = 0;
    virtual void setPort(uint16_t v) = 0;
};

class IPv4Address : public IPAddress {
public:
    typedef std::shared_ptr<IPv4Address> ptr;

    static IPv4Address::ptr Create(const char* address, uint16_t port = 0);

    IPv4Address(uint32_t address = INADDR_ANY, uint16_t port = 0);
    IPv4Address(const sockaddr_in& address);

    const sockaddr *getAddr() const override;
    sockaddr *getAddr() override;
    size_t getAddrLen() const override;
    std::ostream &insert(std::ostream &os) const override;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networkAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) override;

    uint16_t getPort() const override;
    void setPort(uint16_t v) override;
private:
    sockaddr_in m_addr;
};

class IPv6Address : public IPAddress {
public:
    typedef std::shared_ptr<IPv6Address> ptr;

    static IPv6Address::ptr Create(const char* address, uint16_t port = 0);

    IPv6Address();
    IPv6Address(const uint8_t address[16], uint16_t port);
    IPv6Address(const sockaddr_in6& address);

    const sockaddr *getAddr() const override;
    sockaddr *getAddr() override;
    size_t getAddrLen() const override;
    std::ostream &insert(std::ostream &os) const override;

    IPAddress::ptr broadcastAddress(uint32_t prefix_len) override;
    IPAddress::ptr networkAddress(uint32_t prefix_len) override;
    IPAddress::ptr subnetMask(uint32_t prefix_len) override;

    uint16_t getPort() const override;
    void setPort(uint16_t v) override;
private:
    sockaddr_in6 m_addr{};
};

class UnixAddress : public Address {
public:
    typedef std::shared_ptr<UnixAddress> ptr;
    UnixAddress(const std::string &path);
    UnixAddress();

    const sockaddr *getAddr() const override;
    sockaddr *getAddr() override;
    size_t getAddrLen() const override;
    void setAddrLen(size_t v);
    std::ostream &insert(std::ostream &os) const override;
private:
    sockaddr_un m_addr;
    socklen_t m_length;
};

class UnknownAddress : public Address {

public:
    typedef std::shared_ptr<UnknownAddress> ptr;
    UnknownAddress(int family);
    UnknownAddress(const sockaddr& addr);

    const sockaddr *getAddr() const override;
    sockaddr *getAddr() override;
    size_t getAddrLen() const override;
    std::ostream &insert(std::ostream &os) const override;

private:
    sockaddr m_addr;
};
} // namespace solar

#endif // SOLAR_ADDRESS_H
