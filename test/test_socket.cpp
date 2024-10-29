//
// Created by guo on 2025/7/27.
//

#include <gtest/gtest.h>
#include "Core/IOManager.h"
#include "Network/Socket.h"
#include "Log/Log.h"

static solar::Logger::ptr g_logger = SOLAR_LOG_ROOT();

void test_host_and_port() {
    solar::IPAddress::ptr addr = solar::Address::LookUpAnyIpAddress("www.baidu.com");
    EXPECT_TRUE(addr);
    if (!addr) {
        return;
    }
    solar::Socket::ptr sock = solar::Socket::CreateTCPSocket();
    EXPECT_TRUE(sock);
    addr->setPort(80);
    SOLAR_LOG_DEBUG(g_logger) << addr->toString();
    bool connected = sock->connect(addr);
    EXPECT_TRUE(connected);
    if (!connected) {
        return;
    }
    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    EXPECT_GT(rt, 0);
    if (rt <= 0) {
        return;
    }
    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());
    EXPECT_GT(rt, 0);
    if (rt > 0) {
        buffs.resize(rt);
        SOLAR_LOG_INFO(g_logger) << buffs;
    }
}


void test_host_and_service() {
    solar::IPAddress::ptr addr = solar::Address::LookUpAnyIpAddress("www.baidu.com:80");
    EXPECT_TRUE(addr);
    if (!addr) {
        return;
    }
    solar::Socket::ptr sock = solar::Socket::CreateTCPSocket();
    EXPECT_TRUE(sock);
    SOLAR_LOG_DEBUG(g_logger) << addr->toString();
    bool connected = sock->connect(addr);
    EXPECT_TRUE(connected);
    if (!connected) {
        return;
    }
    const char buff[] = "GET / HTTP/1.0\r\n\r\n";
    int rt = sock->send(buff, sizeof(buff));
    EXPECT_GT(rt, 0);
    if (rt <= 0) {
        return;
    }
    std::string buffs;
    buffs.resize(4096);
    rt = sock->recv(&buffs[0], buffs.size());
    EXPECT_GT(rt, 0);
    if (rt > 0) {
        buffs.resize(rt);
        SOLAR_LOG_INFO(g_logger) << buffs;
    }
}

// 如果开了 vpn 会导致接收的消息中出现：writing response to :80: reading HTTP GET: unexpected EOF
TEST(TestSocket, host_and_port) {
    solar::IOManager iom;
    iom.schedule(test_host_and_port);
}

// 如果开了 vpn 会导致接收的消息中出现：writing response to :80: reading HTTP GET: unexpected EOF
TEST(TestSocket, host_and_service) {
    solar::IOManager iom;
    iom.schedule(test_host_and_service);
}
