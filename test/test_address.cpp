//
// Created by guo on 2025/7/26.
//
#include <gtest/gtest.h>
#include "Log/Log.h"
#include "Network/Address.h"

static solar::Logger::ptr g_logger = SOLAR_LOG_ROOT();

TEST(TestAddress, LookUp_website) {
    std::vector<solar::Address::ptr> addrs;
    bool v = solar::Address::LookUp(addrs, "www.baidu.com");
    EXPECT_TRUE(v) << "lookup fail";
    EXPECT_FALSE(addrs.empty());
    for (size_t i = 0; i < addrs.size(); ++i) {
        SOLAR_LOG_INFO(g_logger) << i << " - " << addrs[i]->toString();
    }
}

TEST(TestAddress, LookUp_port) {
    std::vector<solar::Address::ptr> addrs;
    bool v = solar::Address::LookUp(addrs, "www.baidu.com:80");
    EXPECT_TRUE(v) << "lookup fail";
    EXPECT_FALSE(addrs.empty());
    for (size_t i = 0; i < addrs.size(); ++i) {
        SOLAR_LOG_INFO(g_logger) << i << " - " << addrs[i]->toString();
    }
}

TEST(TestAddress, LookUp_http_service) {
    std::vector<solar::Address::ptr> addrs;
    bool v = solar::Address::LookUp(addrs, "www.baidu.com:http");
    EXPECT_TRUE(v) << "lookup fail";
    EXPECT_FALSE(addrs.empty());
    for (size_t i = 0; i < addrs.size(); ++i) {
        SOLAR_LOG_INFO(g_logger) << i << " - " << addrs[i]->toString();
    }
}

TEST(TestAddress, LookUp_ftp_service) {
    std::vector<solar::Address::ptr> addrs;
    bool v = solar::Address::LookUp(addrs, "www.baidu.com:ftp");

    EXPECT_TRUE(v) << "lookup fail";
    EXPECT_FALSE(addrs.empty());
    for (size_t i = 0; i < addrs.size(); ++i) {
        SOLAR_LOG_INFO(g_logger) << i << " - " << addrs[i]->toString();
    }
}

TEST(TestAddress, iface) {
    std::multimap<std::string, std::pair<solar::Address::ptr, uint32_t>>  results;

    bool v = solar::Address::GetInterfaceAddresses(results);

    EXPECT_TRUE(v);
    EXPECT_FALSE(results.empty());
    for (auto& i : results) {
        SOLAR_LOG_INFO(g_logger) << i.first << " - " << i.second.first->toString() << " - " << i.second.second;
    }
}

TEST(TestAddress, ipv4_host) {
    auto addr = solar::IPAddress::Create("www.baidu.com");
    EXPECT_TRUE(addr);
    if (addr) {
        SOLAR_LOG_INFO(g_logger) << addr->toString();
    }
}

TEST(TestAddress, ipv4_numerical) {
    auto addr = solar::IPAddress::Create("192.10.8.8");
    EXPECT_TRUE(addr);
    if (addr) {
        SOLAR_LOG_INFO(g_logger) << addr->toString();
    }
}
