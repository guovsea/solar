//
// Created by guo on 2025/7/27.
//

#include <netinet/tcp.h>

#include "Core/fd_manager.h"
#include "Log/Log.h"
#include "Log/Logger.h"
#include "Util/macro.h"
#include "Core/Hook.h"
#include "Socket.h"

#include "Core/IOManager.h"

namespace solar {

static  Logger::ptr g_logger = SOLAR_LOG_NAME("system");

Socket::ptr Socket::CreateTCP(solar::Address::ptr address) {
    return std::make_shared<Socket>(address->getFamily(), TCP, 0);
}

Socket::ptr Socket::CreateUDP(solar::Address::ptr address) {
    return std::make_shared<Socket>(address->getFamily(), UDP, 0);
}

Socket::ptr Socket::CreateTCPSocket() {
   return std::make_shared<Socket>(IPv4, TCP, 0);
}

Socket::ptr Socket::CreateUDPSocket() {
    return std::make_shared<Socket>(IPv4, UDP, 0);
}

Socket::ptr Socket::CreateTCPSocket6() {
    return std::make_shared<Socket>(IPv6, TCP, 0);
}

Socket::ptr Socket::CreateUDPSocket6() {
    return  std::make_shared<Socket>(IPv6, UDP, 0);
}

Socket::ptr Socket::CreateUnixTCPSocket() {
    return std::make_shared<Socket>(UNIX, TCP, 0);
}

Socket::ptr Socket::CreateUnixUDPSocket() {
    return std::make_shared<Socket>(UNIX, UDP, 0);
}

Socket::Socket(int family, int type, int protocol)
    :m_sock {-1},
    m_family{family},
    m_type{type},
    m_protocol{protocol},
    m_isConnected{false} {
}

Socket::~Socket() {
    close();
}

int64_t Socket::getSendTimeout() {
    FdCtx::ptr ctx = FdMgr::Instance()->get(m_sock);
    if (ctx) {
        return ctx->getTimeout(SO_SNDTIMEO);
    }
    return -1;
}

void Socket::setSendTimeout(int64_t v) {
    struct timeval tv{static_cast<int>(v / 1000), static_cast<int>(v % 1000 * 1000)};
    setOption(SOL_SOCKET, SO_SNDTIMEO, &tv);
}

int64_t Socket::getRecvTimeout() {
    FdCtx::ptr ctx = FdMgr::Instance()->get(m_sock);
    if (ctx) {
        return ctx->getTimeout(SO_SNDTIMEO);
    }
    return -1;
}

void Socket::setRecvTimeout(int64_t v) {
    struct timeval tv{static_cast<int>(v / 1000), static_cast<int>(v % 1000 * 1000)};
    setOption(SOL_SOCKET, SO_SNDTIMEO, tv);
}

bool Socket::getOption(int level, int option, void *result, size_t *len) {
    int rt = getsockopt(m_sock, level, option, result, reinterpret_cast<socklen_t*>(len));
    if (rt) {
        SOLAR_LOG_DEBUG(g_logger) << "getOption sock=" << m_sock
            << " leve=" << level << " option"
            << option << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

bool Socket::setOption(int level, int option, const void *value, size_t len) {
    int rt = setsockopt(m_sock, level, option, value, static_cast<socklen_t>(len));
    if (rt) {
        SOLAR_LOG_DEBUG(g_logger) << "setOption sock=" << m_sock
            << " leve=" << level << " option"
            << option << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

Socket::ptr Socket::accept() {
    Socket::ptr sock = std::make_shared<Socket>(m_family, m_type, m_protocol);
    int newsock = ::accept(m_sock, nullptr, nullptr);
    if (newsock == -1) {
        SOLAR_LOG_ERROR(g_logger) << "accept(" << m_sock << ") errno="
            << errno << " errstr=" << strerror(errno);
        return nullptr;
    }
    if (sock->init(newsock)) {
        return sock;
    }
    return nullptr;
}

bool Socket::bind(const Address::ptr addr) {
    if (!isValid()) {
        newSock();
        if (!isValid()) {
            return false;
        }
    }

    if (addr->getFamily() != m_family) {
        SOLAR_LOG_ERROR(g_logger) << "bind sock.family("
            << m_family << ") addr.family(" << addr->getFamily()
            << ") not equal, addr=" << addr->toString();
        return false;
    }

    if (::bind(m_sock, addr->getAddr(), addr->getAddrLen())) {
        SOLAR_LOG_ERROR(g_logger) << "bind errno=" << errno
            << " strerr=" << strerror(errno);
        return false;
    }
    getLocalAddress();
    return true;
}

bool Socket::connect(const Address::ptr addr, uint64_t timeout_ms) {
    if (!isValid()) {
        newSock();
        if (!isValid()) {
            return false;
        }
    }

    if (addr->getFamily() != m_family) {
        SOLAR_LOG_ERROR(g_logger) << "connect sock.family("
            << m_family << ") addr.family(" << addr->getFamily()
            << ") not equal, addr=" << addr->toString();
        return false;
    }

    if (timeout_ms == ~0ul) {
        if (::connect(m_sock, addr->getAddr(), addr->getAddrLen())) {
            SOLAR_LOG_ERROR(g_logger) << "sock=" << m_sock << " connect(" << addr->toString()
                << ") error errno=" << errno << " errstr=" << strerror(errno);
            close();
            return false;
        }
    } else {
        if (connect_with_timeout(m_sock, addr->getAddr(), addr->getAddrLen(), timeout_ms)) {
            SOLAR_LOG_ERROR(g_logger) << "sock=" << m_sock << " connect(" << addr->toString()
                << ") timeout=" << timeout_ms << "error errno="
                << errno << " errstr" << strerror(errno);
            close();
            return false;
        }
    }
    m_isConnected = true;
    getRemoteAddress();
    getLocalAddress();
    return true;
}

bool Socket::listen(int backlog) {
    if (!isValid()) {
        SOLAR_LOG_ERROR(g_logger) << "listen error sock= -1";
        return false;
    }
    if (::listen(m_sock, backlog)) {
        SOLAR_LOG_ERROR(g_logger) << "listen error errno=" << errno
            << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

bool Socket::close() {
    if (!m_isConnected && m_sock == -1) {
        return true;
    }
    m_isConnected = false;
    if (m_sock != -1) {
        ::close(m_sock);
        m_sock = -1;
        return true;
    }
    return false;
}

int Socket::send(const void *buffer, size_t length, int flags) {
    if (isConnected()) {
        return ::send(m_sock, buffer, length, flags);
    }
    return -1;
}

int Socket::send(const iovec *buffers, size_t length, int flags) {
    if (isConnected()) {
        msghdr msg{};
        msg.msg_iov = const_cast<iovec*>(buffers);
        msg.msg_iovlen = length;
        return ::sendmsg(m_sock, &msg, flags);
    }
    return -1;
}

int Socket::sendTo(const void *buffer, size_t length, const Address::ptr to, int flags) {
    if (!isConnected()) {
        return ::sendto(m_sock, buffer, length, flags, to->getAddr(), to->getAddrLen());
    }
    return -1;
}

int Socket::sendTo(const iovec *buffers, size_t length, const Address::ptr to, int flags) {
    if (isConnected()) {
        msghdr msg{};
        msg.msg_iov = const_cast<iovec*>(buffers);
        msg.msg_iovlen = length;
        msg.msg_name = to->getAddr();
        msg.msg_namelen = to->getAddrLen();
        return ::sendmsg(m_sock, &msg, flags);
    }
    return -1;
}

int Socket::recv(void *buffer, size_t length, int flags) {
    if (isConnected()) {
        return ::recv(m_sock, buffer, length, flags);
    }
    return -1;
}

int Socket::recv(iovec *buffers, size_t length, int flags) {
    if (isConnected()) {
        msghdr msg{};
        msg.msg_iov = const_cast<iovec*>(buffers);
        msg.msg_iovlen = length;
        return ::recvmsg(m_sock, &msg, flags);
    }
    return -1;
}

int Socket::recvFrom(void *buffer, size_t length, Address::ptr from, int flags) {
    if (isConnected()) {
        socklen_t len = from->getAddrLen();
        return ::recvfrom(m_sock, buffer, length, flags, const_cast<sockaddr*>(from->getAddr()), &len);
    }
    return -1;
}

int Socket::recvFrom(iovec *buffers, size_t length, Address::ptr from, int flags) {
    if (isConnected()) {
        msghdr msg{};
        msg.msg_iov = buffers;
        msg.msg_iovlen = length;
        msg.msg_name = from->getAddr();
        msg.msg_namelen = from->getAddrLen();
        return ::recvmsg(m_sock, &msg, flags);
    }
    return -1;
}

Address::ptr Socket::getRemoteAddress() {
    if (m_remoteAddress) {
        return m_remoteAddress;
    }
    Address::ptr result{};
    switch (m_family) {
        case AF_INET:
            result = std::make_shared<IPv4Address>();
            break;
        case AF_INET6:
            result = std::make_shared<IPv6Address>();
            break;
        case AF_UNIX:
            result = std::make_shared<UnixAddress>();
            break;
        default:
            result = std::make_shared<UnknownAddress>(m_family);
            break;
    }
    socklen_t addrlen = result->getAddrLen();
    if (::getpeername(m_sock, result->getAddr(), &addrlen)) {
        SOLAR_LOG_ERROR(g_logger) << "getpeername error sock=" << m_sock
            << " errno=" << errno << " errstr=" << strerror(errno);
        return std::make_shared<UnknownAddress>(m_family);
    }
    if (m_family == AF_UNIX) {
        UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
        addr->setAddrLen(addrlen);
    }
    m_remoteAddress = result;
    return m_remoteAddress;

}

Address::ptr Socket::getLocalAddress() {
    if (m_localAddress) {
        return m_localAddress;
    }
    Address::ptr result{};
    switch (m_family) {
        case AF_INET:
            result = std::make_shared<IPv4Address>();
            break;
        case AF_INET6:
            result = std::make_shared<IPv6Address>();
            break;
        case AF_UNIX:
            result = std::make_shared<UnixAddress>();
            break;
        default:
            result = std::make_shared<UnknownAddress>(m_family);
            break;
    }
    socklen_t addrlen = result->getAddrLen();
    if (::getsockname(m_sock, result->getAddr(), &addrlen)) {
        SOLAR_LOG_ERROR(g_logger) << "getsockname error sock=" << m_sock
            << " errno=" << errno << " errstr=" << strerror(errno);
        return std::make_shared<UnknownAddress>(m_family);
    }
    if (m_family == AF_UNIX) {
        UnixAddress::ptr addr = std::dynamic_pointer_cast<UnixAddress>(result);
        addr->setAddrLen(addrlen);
    }
    m_localAddress = result;
    return m_localAddress;
}

bool Socket::isValid() const {
    return m_sock != -1;
}

int Socket::getError() {
    int error = 0;
    size_t len = sizeof(error);
    if (!getOption(SOL_SOCKET, SO_ERROR, &error, &len)) {
        return -1;
    }
    return error;
}

std::ostream & Socket::dump(std::ostream &os) const {
    os << "[Socket sock=" << m_sock
        << " is_connected=" << m_isConnected
        << " family=" << m_family
        << " type=" << m_type
        << " protocol=" << m_protocol;
    if (m_localAddress) {
        os << " local_address=" << m_localAddress->toString();
    }
    if (m_remoteAddress) {
        os << "remote_address=" << m_remoteAddress->toString();
    }
    os << "]";
    return os;
}

bool Socket::cancelRead() {
    return IOManager::GetThis()->cancelEvent(m_sock, solar::IOManager::READ);
}

bool Socket::cancelWrite() {
    return IOManager::GetThis()->cancelEvent(m_sock, solar::IOManager::WRITE);
}

bool Socket::cancelAccept() {
    return IOManager::GetThis()->cancelEvent(m_sock, solar::IOManager::READ);
}

bool Socket::cancelAll() {
    return IOManager::GetThis()->cancelAll(m_sock);
}

void Socket::initSock() {
    int val = 1;
    setOption(SOL_SOCKET, SO_REUSEADDR, val);
    if (m_type == SOCK_STREAM) {
        setOption(IPPROTO_TCP, TCP_NODELAY, val);
    }
}

void Socket::newSock() {
    m_sock = socket(m_family, m_type, m_protocol);
    if (m_sock != -1) {
        initSock();
    }  else {
        SOLAR_LOG_ERROR(g_logger) << "socket(" << m_family
            << ", " << m_type << ", " << m_protocol << ") errno="
            << errno << " errstr=" << strerror(errno);
    }
}

bool Socket::init(int sock) {
    FdCtx::ptr ctx = FdMgr::Instance()->get(sock);
    if (ctx && ctx->isSocket() && !ctx->isClosed()) {
        m_sock = sock;
        m_isConnected = true;
        initSock();
        getLocalAddress();
        getRemoteAddress();
        return true;
    }
    return false;
}

}
