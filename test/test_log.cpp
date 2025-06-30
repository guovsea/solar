#include <gtest/gtest.h>

#include "Log/Log.h"
#include "Util/Config.h"
#include "Util/Util.h"
#include "test_dir.h"

#if 1
// Demonstrate some basic assertions.
TEST(TestLog, TestLog) {
  solar::Logger::ptr pLogger(new solar::Logger);
  pLogger->addAppender(solar::LogAppender::ptr(new solar::StdoutLogAppender));
  solar::LogEvent::ptr pEvent(
      new solar::LogEvent(pLogger, solar::LogLevel::DEBUG, __FILE__, __LINE__,
                          0, solar::GetThreadId(), solar::GetFiberId(), time(0),
                          solar::Thread::GetName()));
  pEvent->getSS() << "hello solar log";
  pLogger->log(solar::LogLevel::DEBUG, pEvent);

  SOLAR_LOG_DEBUG(pLogger) << "log debug";
  SOLAR_LOG_INFO(pLogger) << "log info";
  SOLAR_LOG_FMT_DEBUG(pLogger, "%s %s", "log", "debug");

  auto l = solar::LoggerMgr::Instance()->getLogger("XX");
  SOLAR_LOG_DEBUG(l) << "hello loggerMgr";

  SOLAR_LOG_DEBUG(SOLAR_LOG_ROOT()) << "log root";
}

TEST(TestLog, LoggerWithFileAppender) {
  SOLAR_LOG_ROOT()->addAppender(
      std::make_shared<solar::FileLogAppender>("test.log"));
  SOLAR_LOG_DEBUG(SOLAR_LOG_NAME("LoggerWithFileAppender")) << "hello world";
}

TEST(TestLog, ToYamlString) {
  std::stringstream ss;
  ss << SOLAR_LOG_NAME("xxx")->toYamlString();
  std::cout << ss.str();
  std::string log_yaml = R"(name: xxx
level: DEBUG
formatter: "%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%F%T[%p]%T[%c]%T%f:%l%T%m%n")";
  EXPECT_EQ(ss.str(), log_yaml);
}

TEST(TestLog, InitLogSystemWithYAML) {
  YAML::Node node = YAML::LoadFile(TEST_DIR + "test_log.yml");
  solar::Config::LoadFromYaml(node);
  std::stringstream ss;
  ss << SOLAR_LOG_NAME("test_logger")->toYamlString();
  std::cout << ss.str();
  std::string logger_mgr_updated_yaml = R"(name: test_logger
level: INFO
formatter: "%d%T%m%n"
appenders:
  - type: FileLogAppender
    file: test.txt
    formatter: "%d%T%m%n"
  - type: StdoutLogAppender
    formatter: "%d%T%m%n")";
  EXPECT_EQ(ss.str(), logger_mgr_updated_yaml);
}

#endif