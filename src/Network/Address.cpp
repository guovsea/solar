//
// Created by guo on 2025/7/24.
//

#include <cstring>
#include <cmath>
#include <memory>
#include <sstream>

#include  "Util/Endian.h"
#include "Address.h"

namespace solar {

/**
 *p
 * @tparam T
 * @param bits
 * @return 返回掩码的反码
 */
template<typename T>
static T CreateMask(uint32_t bits) {
    return (1 << (sizeof(T) * 8 - bits)) - 1;
}

int Address::getFamily() const { return getAddr()->sa_family; }

std::string Address::toString() const {
    std::stringstream ss;
    insert(ss);
    return ss.str();
}

bool Address::operator<(const Address &rhs) const {
    socklen_t min_len = std::min(getAddrLen(), rhs.getAddrLen());
    const int result = memcmp(getAddr(), rhs.getAddr(), min_len);
    if (result < 0) {
        return true;
    }
    if (result > 0) {
        return false;
    }
    if (getAddrLen() < rhs.getAddrLen()) {
        return true;
    }
    return false;
}

bool Address::operator==(const Address &rhs) const {
    return getAddrLen() == rhs.getAddrLen()
        && memcmp(getAddr(), rhs.getAddr(), getAddrLen()) == 0;
}

bool Address::operator!=(const Address &rhs) const {
    return !(*this == rhs);
}

IPv4Address::IPv4Address(uint32_t address, uint32_t port)
    :m_addr{}
{
    m_addr.sin_family = AF_INET;
    // 只在小端机器上转换为大端，因为网络字节序是大端
    m_addr.sin_port = ByteswapOnLittleEndian(port);
    m_addr.sin_addr.s_addr = ByteswapOnLittleEndian(address);
}

IPv4Address::IPv4Address(const sockaddr_in &address)
    : m_addr{address} {
}

const sockaddr * IPv4Address::getAddr() const {
    return reinterpret_cast<const sockaddr*>(&m_addr);
}

size_t IPv4Address::getAddrLen() const {
    return sizeof(m_addr);
}

std::ostream & IPv4Address::insert(std::ostream &os) const {
    uint32_t addr = ByteswapOnLittleEndian((m_addr.sin_addr.s_addr));
    os << ((addr >> 24) & 0xff) << "."
       << ((addr >> 16) & 0xff) << "."
       << ((addr >> 8) & 0xff) << "."
       << (addr & 0xff);
    os << ":" << ByteswapOnLittleEndian(m_addr.sin_port);
    return os;
}

IPAddress::ptr IPv4Address::broadcastAddress(uint32_t prefix_len) {
    if (prefix_len > 32) {
        return nullptr;
    }
    // IP 地址 =  [network ID + host ID]， broadcast 地址的 host ID 全为 1
    sockaddr_in baddr(m_addr);
    baddr.sin_addr.s_addr |= ByteswapOnLittleEndian(
        CreateMask<uint32_t>(prefix_len));
    return std::make_shared<IPv4Address>(baddr);
}

IPAddress::ptr IPv4Address::networkAddress(uint32_t prefix_len) {
    sockaddr_in network{};
    network.sin_family = AF_INET;
    network.sin_addr.s_addr &= ~ByteswapOnLittleEndian(
        CreateMask<uint32_t>(prefix_len));
    return std::make_shared<IPv4Address>(network);
}

IPAddress::ptr IPv4Address::subnetMask(uint32_t prefix_len) {
    sockaddr_in subnet{};
    subnet.sin_family = AF_INET;
    subnet.sin_addr.s_addr = ~ByteswapOnLittleEndian(CreateMask<uint32_t>(prefix_len));
    return std::make_shared<IPv4Address>(subnet);
}

uint32_t IPv4Address::getPort() const {
    return ByteswapOnLittleEndian(m_addr.sin_port);
}

void IPv4Address::setPort(uint32_t v) {
    m_addr.sin_port = ByteswapOnLittleEndian(v);
}

IPv6Address::IPv6Address()
    :m_addr{}
{
    m_addr.sin6_family = AF_INET6;
}

IPv6Address::IPv6Address(const char *address, uint32_t port)
    :m_addr{}
{
    m_addr.sin6_family = AF_INET6;
    m_addr.sin6_port = ByteswapOnLittleEndian(port);
    memcpy(&m_addr.sin6_addr.s6_addr, address, 16);
}

IPv6Address::IPv6Address(const sockaddr_in6 &address)
    :m_addr{address}
{
}

const sockaddr * IPv6Address::getAddr() const {
    return reinterpret_cast<const sockaddr*>(&m_addr);
}

size_t IPv6Address::getAddrLen() const {
    return sizeof(m_addr);
}

std::ostream & IPv6Address::insert(std::ostream &os) const {
    os << "[";
    const uint16_t* addr = reinterpret_cast<const uint16_t*>(m_addr.sin6_addr.s6_addr);
    bool used_zeros = false;
    for (size_t i = 0; i < 8; ++i) {
        if (addr[i] == 0 && !used_zeros) {
            continue;
        }
        if (i && addr[i - 1] == 0 && !used_zeros) {
            os << ":";
            used_zeros = true;
        }
        if (i != 0) {
            os << ":";
        }
        os << std::hex << static_cast<int>(ByteswapOnLittleEndian(addr[i])) << std::dec;
        if (!used_zeros && addr[7] == 0) {
            os << "::";
        }
        os << "]:" << ByteswapOnLittleEndian(m_addr.sin6_port);
    }
    return os;
}

IPAddress::ptr IPv6Address::broadcastAddress(uint32_t prefix_len) {
    sockaddr_in6 baddr{m_addr};
    // IP 地址 =  [network ID + host ID]， broadcast 地址的 host ID 全为 1
    // 先处理除主机号最前面没有占到一个字节的比特位
    baddr.sin6_addr.s6_addr[prefix_len / 8] |= CreateMask<uint8_t>(prefix_len % 8);
    // 再处理主机号后面若干个字节
    for (size_t i = prefix_len / 8 + 1; i < 16; ++i) {
        baddr.sin6_addr.s6_addr[i] = 0xff;
    }
    return std::make_shared<IPv6Address>(baddr);
}

IPAddress::ptr IPv6Address::networkAddress(uint32_t prefix_len) {
    sockaddr_in6 network{m_addr};
    network.sin6_addr.s6_addr[prefix_len / 8] &= ~CreateMask<uint8_t>(prefix_len % 8);
    for (size_t i = prefix_len / 8 + 1; i < 16; ++i) {
        network.sin6_addr.s6_addr[i] = 0x0;
    }
    return std::make_shared<IPv6Address>(network);
}

IPAddress::ptr IPv6Address::subnetMask(uint32_t prefix_len) {
    sockaddr_in6 subnet{};
    subnet.sin6_family = AF_INET6;
    for (uint32_t i = 0; i < prefix_len / 8; ++i) {
        subnet.sin6_addr.s6_addr[i] = 0xff;
    }
    subnet.sin6_addr.s6_addr[prefix_len / 8] = ~CreateMask<uint8_t>(prefix_len % 8);
    return std::make_shared<IPv6Address>(subnet);
}

uint32_t IPv6Address::getPort() const {
    return ByteswapOnLittleEndian(m_addr.sin6_port);
}

void IPv6Address::setPort(uint32_t v){
    m_addr.sin6_port = ByteswapOnLittleEndian(v);
}

// 末尾需要 '\0'，因此需要 -1
constexpr size_t MAX_PATH_LEN = sizeof(static_cast<sockaddr_un*>(nullptr)->sun_path) - 1;

UnixAddress::UnixAddress(const std::string &path)
    :m_addr{}
{
    m_addr.sun_family = AF_UNIX;
    m_length = path.size() + 1;

    if (!path.empty() && path[0] == '\0') {
        --m_length;
    }
    if (m_length > sizeof(m_addr.sun_path)) {
        throw std::logic_error("path too long");
    }
    memcpy(&m_addr.sun_path, path.c_str(), m_length);
    // struct sockaddr_un
    // {
    //     __SOCKADDR_COMMON (sun_);
    //     char sun_path[108];		/* Path name.  */
    // };
    // 总长度要加上 sun_path 之前的结构体的大小
    m_length += offsetof(sockaddr_un, sun_path);
}

UnixAddress::UnixAddress()
    :m_addr{} {
    m_addr.sun_family = AF_UNIX;
    m_length = offsetof(sockaddr_un, sun_path) + MAX_PATH_LEN;
}

const sockaddr * UnixAddress::getAddr() const {
    return reinterpret_cast<const sockaddr*>(&m_addr);
}

size_t UnixAddress::getAddrLen() const {
    return m_length;
}

std::ostream & UnixAddress::insert(std::ostream &os) const {
    if (m_length > offsetof(sockaddr_un, sun_path)
        && m_addr.sun_path[0] == '\0') {
        return os << "\\0" << std::string(m_addr.sun_path + 1,
            m_length - offsetof(sockaddr_un, sun_path) - 1);
    }
    return os << m_addr.sun_path;
}

UnknownAddress::UnknownAddress(int family)
    :m_addr{}
{
    m_addr.sa_family = family;
}

const sockaddr * UnknownAddress::getAddr() const {
    return &m_addr;
}

size_t UnknownAddress::getAddrLen() const {
    return sizeof(m_addr);
}

std::ostream & UnknownAddress::insert(std::ostream &os) const {
    os << "[UnknownAddress family=" << m_addr.sa_family << "]";
    return os;
}

} // namespace solar