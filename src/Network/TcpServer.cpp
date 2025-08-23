//
// Created by guo on 2025/8/7.
//

#include "TcpServer.h"
#include "Log/Log.h"
#include "Util/Config.h"

namespace solar {

static Logger::ptr g_logger = SOLAR_LOG_NAME("system");

static ConfigVar<uint64_t>::ptr g_tcp_server_read_timeout =
    Config::Lookup("tcp_server.read_timeout", (uint64_t)(60 * 1000 * 2)); // 单位 ms

TcpServer::TcpServer(IOManager *worker, IOManager *accept_worker)
    :m_worker{worker}
    ,m_acceptWorker{accept_worker}
    ,m_recvTimeout{g_tcp_server_read_timeout->getValue()}
    ,m_name{"solar/1.0.0"}
    ,m_isStop{true} {
}

solar::TcpServer::~TcpServer() {
}

bool TcpServer::bind(Address::ptr addr) {
    std::vector<Address::ptr> addrs;
    addrs.push_back(addr);
    std::vector<Address::ptr> fails;
    return bind(addrs, fails);
}

bool TcpServer::bind(const std::vector<Address::ptr>& addrs, std::vector<Address::ptr>& fails) {
    for (auto& addr : addrs) {
        Socket::ptr sock = Socket::CreateTCP(addr);
        if (!sock->bind(addr)) {
            SOLAR_LOG_ERROR(g_logger) << "bind fail errno="
                << errno << " errstr=" << strerror(errno)
                << " addr=[" << addr->toString() << "]";
            continue;
        }
        if (!sock->listen()) {
            SOLAR_LOG_ERROR(g_logger) << "listen fail errno="
                << errno << " errstr=" << strerror(errno)
                << " addr=[" << addr->toString() << "]";
            fails.push_back(addr);
            continue;
        }
        m_socks.push_back(sock);
    }
    if (!fails.empty()) {
        m_socks.clear();
        return false;
    }
    for (auto& i: m_socks) {
        SOLAR_LOG_INFO(g_logger) << "server bind success: " << *i;
    }
    return true;
}

bool TcpServer::start() {
    if (!m_isStop) {
        return true;
    }
    m_isStop = false;
    for (auto& sock : m_socks) {
        m_acceptWorker->schedule(std::bind(&TcpServer::startAccept, shared_from_this(), sock));
    }
    return true;
}

bool TcpServer::stop() {
    m_isStop = true;
    auto self = shared_from_this(); // 防止被析构
    m_acceptWorker->schedule([this, self]() {
       for (auto & sock : m_socks) {
           sock->cancelAll();
           sock->close();
       }
        m_socks.clear();
    });
    return true;
}

void TcpServer::handleClient(Socket::ptr client) {
    SOLAR_LOG_INFO(g_logger) << "handleClient:" << *client;
}

void TcpServer::startAccept(Socket::ptr sock) {
    while (!m_isStop) {
        Socket::ptr client = sock->accept();
        if (client) {
            client->setRecvTimeout(m_recvTimeout);
            m_worker->schedule(std::bind(&TcpServer::handleClient, shared_from_this(), client));
        }
        else {
            SOLAR_LOG_ERROR(g_logger) << "accept errno=" << errno
                << " errstr=" << strerror(errno);
        }
    }
}
}
