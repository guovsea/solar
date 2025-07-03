#include "Core/Scheduler.h"
#include "Log/Log.h"
#include <gtest/gtest.h>

namespace {
solar::Logger::ptr g_logger = SOLAR_LOG_NAME("root");
}

TEST(TestScheduler, TestScheduler) {
  solar::Scheduler scheduler{};
  scheduler.start();
  scheduler.stop();
}