//
// Created by guo on 2025/7/24.
//

#ifndef SOLAR_ADDRESS_H
#define SOLAR_ADDRESS_H
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>

namespace solar {

class Address {
public:
    typedef std::shared_ptr<Address> ptr;
    virtual ~Address() {}

    int getFamily() const;

    virtual const sockaddr *getAddr() const = 0;
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

    virtual ~IPAddress() {}

    virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len);
    virtual IPAddress::ptr networkAddress(uint32_t prefix_len);
    virtual IPAddress::ptr subnetMask(uint32_t prefix_len);

    virtual uint32_t getPort() const = 0;
    virtual void setPort(uint32_t v);
};

class IPv4Address : public IPAddress {
public:
    typedef std::shared_ptr<IPv4Address> ptr;
    IPv4Address(uint32_t address = INETADD_ANY, uint32_t port = 0);

    virtual const sockaddr *getAddr() const = 0;
    virtual size_t getAddrLen() const = 0;
    virtual std::ostream &insert(std::ostream &os) const;

    virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len);
    virtual IPAddress::ptr networkAddress(uint32_t prefix_len);

    virtual IPAddress::ptr subnetMask(uint32_t prefix_len);

    virtual uint32_t getPort() const = 0;
    virtual void setPort(uint32_t v);

private:
    sockaddr_in m_addr;
};

class IPv6Address : public IPAddress {
public:
    typedef std::shared_ptr<IPv4Address> ptr;
    IPv6Address(uint32_t address = INETADD_ANY, uint32_t port = 0);

    virtual const sockaddr *getAddr() const = 0;
    virtual size_t getAddrLen() const = 0;
    virtual std::ostream &insert(std::ostream &os) const;

    virtual IPAddress::ptr broadcastAddress(uint32_t prefix_len);
    virtual IPAddress::ptr networkAddress(uint32_t prefix_len);

    virtual IPAddress::ptr subnetMask(uint32_t prefix_len);

    virtual uint32_t getPort() const = 0;
    virtual void setPort(uint32_t v);

private:
    sockaddr_in6 m_addr;
};

class UinxAddress : public Address {
public:
    typedef std::shared_ptr<UinxAddress> ptr;
    UnixAddress(const std::string &path);

    virtual const sockaddr *getAddr() const = 0;
    virtual size_t getAddrLen() const = 0;
    virtual std::ostream &insert(std::ostream &os) const;

private:
    sockaddr_un m_addr;
    socklen m_length;
};

class UnkownAddress : public Address {

public:
    typedef std::shared_ptr<UnkownAddress> ptr;
    UnkownAddress(const std::string &path);

    virtual const sockaddr *getAddr() const = 0;
    virtual size_t getAddrLen() const = 0;
    virtual std::ostream &insert(std::ostream &os) const;

private:
    sockaddr m_addr;
};

} // namespace solar

#endif // SOLAR_ADDRESS_H
