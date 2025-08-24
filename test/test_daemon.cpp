#include <gtest/gtest.h>
#include "Core/daemon.h"
#include "Core/IOManager.h"
#include "Log/Log.h"

static solar::Logger::ptr g_logger = SOLAR_LOG_NAME("root");

static std::shared_ptr<solar::Timer> timer1;
int server_main1(int argc, char* argv[]) {
    SOLAR_LOG_INFO(g_logger) << solar::ProcessInfoMgr::Instance()->toString();
    solar::IOManager iom{ 1 };
    timer1 = iom.addTimer(500, []() {
        SOLAR_LOG_INFO(g_logger) << "onTimer";
        static int count = 0;
        if (++count >= 5) {
            timer1->cancel();
        }
    }, true);
    return 0;
}


TEST(test_daemon, no_daemon) {
    int argc = 1;
    char* argv[] = { (char*)"server_main"};
    int rt = solar::start_daemon(argc, argv, server_main1, false);
    EXPECT_EQ(rt, 0);
}

static std::shared_ptr<solar::Timer> timer2;
int server_main2(int argc, char* argv[]) {
    SOLAR_LOG_INFO(g_logger) << solar::ProcessInfoMgr::Instance()->toString();
    solar::IOManager iom{ 1 };
    timer1 = iom.addTimer(500, []() {
        SOLAR_LOG_INFO(g_logger) << "onTimer";
        static int count = 0;
        if (++count >= 5) {
            timer1->cancel();
        }
    }, true);
    return 0;
}

TEST(test_daemon, daemon) {
    GTEST_SKIP();
    int argc = 2;
    char* argv[] = { (char*)"server_main"};
    int rt = solar::start_daemon(argc, argv, server_main2, true);
    EXPECT_EQ(rt, 0);
}