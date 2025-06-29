#include "Core/Fiber.h"
#include "Log/Log.h"
#include <gtest/gtest.h>

namespace {
// 防止单元测试时时打印过多干扰信息
solar::Logger::ptr g_logger = SOLAR_LOG_NAME("null_logger");

std::stringstream ss;
} // namespace

namespace solar {
void run_in_fiber() {
  ss << "run_in_fiber begin\n";
  SOLAR_LOG_INFO(g_logger) << "run_in_fiber begin";
  solar::Fiber::YeildToHold();
  SOLAR_LOG_INFO(g_logger) << "run_in_fiber end";
  ss << "run_in_fiber end\n";
  solar::Fiber::YeildToHold();
}

TEST(TestFiber, TestSwapOut) {
  {
    solar::Fiber::GetThis();
    ss << "main begin\n";
    SOLAR_LOG_INFO(g_logger) << "main begin";
    solar::Fiber::ptr fiber = std::make_shared<Fiber>(run_in_fiber);
    fiber->swapIn();
    SOLAR_LOG_INFO(g_logger) << "main after swapIn";
    ss << "main after swapIn\n";
    fiber->swapIn();
    SOLAR_LOG_INFO(g_logger) << "main after end";
    ss << "main after end\n";
    fiber->swapIn();
    // fiber 对象析构
  }
  std::string result = R"(main begin
run_in_fiber begin
main after swapIn
run_in_fiber end
main after end
)";
  EXPECT_EQ(ss.str(), result);
  SOLAR_LOG_INFO(g_logger) << "main after end 2";
}
} // namespace solar