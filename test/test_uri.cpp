#include <Log/Log.h>
#include <gtest/gtest.h>

#include "Network/Uri.h"

static solar::Logger::ptr g_logger = SOLAR_LOG_ROOT();

TEST(test_uir, test) {
    solar::Uri::ptr uri = solar::Uri::Create("http://www.baidu.com/test/uri?id=100&name=solar#fragment");
    EXPECT_TRUE(uri);
    SOLAR_LOG_INFO(g_logger) << uri->toString();
    auto addr = uri->createAddress();
    EXPECT_TRUE(addr);
    SOLAR_LOG_INFO(g_logger) << *addr;
}

TEST(test_uir, chinese_char) {
    solar::Uri::ptr uri = solar::Uri::Create("http://www.baidu.com/test/中文/uri?id=100&name=solar#fragment");
    EXPECT_TRUE(uri);
    SOLAR_LOG_INFO(g_logger) << uri->toString();
    auto addr = uri->createAddress();
    EXPECT_TRUE(addr);
    SOLAR_LOG_INFO(g_logger) << *addr;
}