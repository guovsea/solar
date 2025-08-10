//
// Created by guo on 2025/8/8.
//
#include <gtest/gtest.h>

#include "Network/Address.h"
#include "Network/TcpServer.h"
#include "Core/IOManager.h"
#include "Log/Log.h"

static solar::Logger::ptr g_logger = SOLAR_LOG_ROOT();


static void run() {
    auto addr = solar::IPAddress::LookUpAny("0.0.0.0:8043");
    auto addr2 = std::make_shared<solar::UnixAddress>("/tmp/unix_address");
    std::vector<solar::Address::ptr> addrs;
    addrs.push_back(addr);
    addrs.push_back(addr2);

    solar::TcpServer::ptr tcp_server = std::make_shared<solar::TcpServer>();
    std::vector<solar::Address::ptr> fails;
    while (!tcp_server->bind(addrs, fails)) {
        sleep(2);
    }
    tcp_server->start();
}

TEST(test_tcp_server, test) {
    solar::IOManager iom{2};
    iom.schedule(run);
}
