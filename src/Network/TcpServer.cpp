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

TcpServer::TcpServer(IOManager *worker)
    :m_worker{worker}
    ,m_readTimeout{g_tcp_server_read_timeout->getValue()}
    ,m_name{"solar/1.0.0"}
    ,m_isStop{true} {
}

solar::TcpServer::~TcpServer() {
}

bool TcpServer::bind(Address::ptr addr) {
    std::vector<Address::ptr> addrs;
    addrs.push_back(addr);
    return bind(addrs);
}

bool TcpServer::bind(const std::vector<Address::ptr>& addrs, std::vector<Address::ptr>& fails) {
    bool rt = true;
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
}

bool TcpServer::stop() {
}

void TcpServer::handleClient(Socket::ptr client) {
}
}
