#include <gtest/gtest.h>
#include "Core/Scheduler.h"
#include "Log/Log.h"

namespace {
solar::Logger::ptr g_logger = SOLAR_LOG_NAME("root");
void test_fiber() {
    static int s_count = 5;
    sleep(1);
    if (--s_count > 0) {
        solar::Scheduler::GetThis()->schedule(&test_fiber);
    }
    SOLAR_LOG_INFO(g_logger) << "test in fiber"
                             << "s_count = " << s_count;
}
} // namespace

TEST(TestScheduler, TestScheduler) {
    GTEST_SKIP();
    solar::Scheduler scheduler{3, true, "test"};
    scheduler.start();
    sleep(2);
    SOLAR_LOG_INFO(g_logger) << "schedule";
    scheduler.schedule(&test_fiber);
    scheduler.stop();
    SOLAR_LOG_INFO(g_logger) << "over";
}

// TODO: More test