#include "Core/Hook.h"
#include "Core/IOManager.h"
#include "Log/Log.h"
#include "Util/Util.h"
#include <gtest/gtest.h>

static solar::Logger::ptr g_logger = SOLAR_LOG_NAME("root");
void test_sleep() {}

TEST(TestHook, only_sleep_one_fiber) {
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
