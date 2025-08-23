#include <arpa/inet.h>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include "Core/IOManager.h"
#include "Log/Log.h"

namespace {
solar::Logger::ptr g_logger = SOLAR_LOG_NAME("root");
}

void test_fiber() { SOLAR_LOG_INFO(g_logger) << "test_fiber"; }

TEST(TestIOManager, test_epoll_wait_and_addEvent) {
    GTEST_SKIP();
    solar::IOManager iom(2);
    iom.schedule(&test_fiber);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(sock, F_SETFL, O_NONBLOCK);

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("183.2.172.177");
    addr.sin_port = htons(80);

    iom.addEvent(sock, solar::IOManager::READ, []() { SOLAR_LOG_INFO(g_logger) << "read call back"; });
    iom.addEvent(sock, solar::IOManager::WRITE, [=]() {
        SOLAR_LOG_INFO(g_logger) << "write call back";
        // 将会调用 read call back
        solar::IOManager::GetThis()->cancelEvent(sock, solar::IOManager::READ);
    });
    connect(sock, (struct sockaddr *) &addr, sizeof(addr));
}

namespace {
bool time_out{false};
}
TEST(TestIOManager, test_add_timer) {
    solar::IOManager iom{2};
    iom.addTimer(
            1000, []() { time_out = true; }, false);
    sleep(2);
    EXPECT_TRUE(time_out);
}

namespace {
int cnt{0};
}
TEST(TestIOManager, test_calcel_timer) {
    solar::IOManager iom(2);
    solar::Timer::ptr counter = iom.addTimer(
            500,
            [&counter]() {
                if (++cnt == 5) {
                    counter->cancel();
                }
            },
            true);
    sleep(3);
    EXPECT_EQ(cnt, 5);
}