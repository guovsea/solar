#include "Core/Scheduler.h"
#include "Log/Log.h"
#include <gtest/gtest.h>

namespace {
solar::Logger::ptr g_logger = SOLAR_LOG_NAME("root");
void test_fiber() { SOLAR_LOG_INFO(g_logger) << "test in fiber"; }
} // namespace

TEST(TestScheduler, TestScheduler) {
  solar::Scheduler scheduler{};
  scheduler.schedule(&test_fiber);
  scheduler.start();
  SOLAR_LOG_INFO(g_logger) << "schedule";
  scheduler.stop();
  SOLAR_LOG_INFO(g_logger) << "over";
}