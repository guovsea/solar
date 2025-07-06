#include "Core/Fiber.h"
#include "Log/Log.h"
#include <gtest/gtest.h>

namespace {
// 防止单元测试时时打印过多干扰信息
solar::Logger::ptr g_logger = SOLAR_LOG_NAME("root");

std::stringstream ss;
} // namespace

namespace solar {
void run_in_fiber() {
  ss << "run_in_fiber begin\n";
  SOLAR_LOG_INFO(g_logger) << "run_in_fiber begin";
  // solar::Fiber::YeildToHold();
  solar::Fiber::GetThis()->back();
  SOLAR_LOG_INFO(g_logger) << "run_in_fiber end";
  ss << "run_in_fiber end\n";
  // solar::Fiber::YeildToHold();
  solar::Fiber::GetThis()->back();
}

void test_fiber() {
  {
    solar::Fiber::GetThis();
    ss << "main begin\n";
    SOLAR_LOG_INFO(g_logger) << "main begin";
    solar::Fiber::ptr fiber = std::make_shared<Fiber>(run_in_fiber);
    fiber->call();
    SOLAR_LOG_INFO(g_logger) << "main after call";
    ss << "main after call\n";
    fiber->call();
    SOLAR_LOG_INFO(g_logger) << "main after end";
    ss << "main after end\n";
    fiber->call();
    // fiber 对象析构
  }
  SOLAR_LOG_INFO(g_logger) << "main after end 2";
}

TEST(TestFiber, TestSwapOut) {
  GTEST_SKIP(); // 引入 Scheduler 后， Fiber 就不能这样使用了
  test_fiber();
  std::string result = R"(main begin
run_in_fiber begin
main after call
run_in_fiber end
main after end
)";
  EXPECT_EQ(ss.str(), result);
}

TEST(TestFiber, TestFiberInMutiThreads) {
  GTEST_SKIP(); // 引入 Scheduler 后， Fiber 就不能这样使用了
  std::vector<solar::Thread::ptr> thrs;
  for (int i = 0; i < 3; ++i) {
    thrs.push_back(std::make_shared<solar::Thread>(
        test_fiber, "thread_" + std::to_string(i)));
  }
  for (auto i : thrs) {
    i->join();
  }
}
} // namespace solar