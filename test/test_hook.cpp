#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include "Core/Hook.h"
#include "Core/IOManager.h"
#include "Log/Log.h"
#include "Util/Util.h"

static solar::Logger::ptr g_logger = SOLAR_LOG_NAME("root");

TEST(TestHook, only_sleep_one_fiber) {
    GTEST_SKIP();
    solar::IOManager iom{1};
    uint64_t now_ms = solar::GetCurrentMS();
    iom.schedule([]() {
        sleep(1);
        SOLAR_LOG_INFO(g_logger) << "sleep 1";
    });
    iom.schedule([now_ms]() {
        sleep(2);
        SOLAR_LOG_INFO(g_logger) << "sleep 2";
        uint64_t after_ms = solar::GetCurrentMS();
        EXPECT_LE(after_ms - now_ms, 3 * 1000);
    });
    SOLAR_LOG_INFO(g_logger) << "test_sleep";
}

void test_connect() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("183.2.172.177");
    addr.sin_port = htons(80);
    SOLAR_LOG_INFO(g_logger) << "begin connect";
    int rt = connect(sock, (struct sockaddr *) &addr, sizeof(addr));
    SOLAR_LOG_INFO(g_logger) << "connect rt=" << rt << " errno=" << errno;
    if (rt) {
        return;
    }
    const char data[] = "GET / HTTP/1.0\r\n\r\n";
    rt = send(sock, data, sizeof(data), 0);
    SOLAR_LOG_INFO(g_logger) << "send rt=" << rt << " errno=" << errno;
    if (rt <= 0) {
        return;
    }
    std::string buf;
    buf.resize(4096);

    rt = recv(sock, &buf[0], buf.size(), 0);
    SOLAR_LOG_INFO(g_logger) << "recv rt=" << rt << " errno=" << errno;
    if (recv <= 0) {
        return;
    }
    buf.resize(rt);
    SOLAR_LOG_INFO(g_logger) << buf;
}

TEST(TestHook, test_connect) {
    solar::IOManager iom;
    iom.schedule(test_connect);
}