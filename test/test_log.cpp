#include <gtest/gtest.h>

#include "Log/Log.h"
#include "Util/Config.h"
#include "Util/Util.h"
#include "test_util.h"

#if 1
// Demonstrate some basic assertions.
TEST(TestLog, TestLog) {
  solar::Logger::ptr pLogger(new solar::Logger);
  pLogger->addAppender(solar::LogAppender::ptr(new solar::StdoutLogAppender));
  solar::LogEvent::ptr pEvent(new solar::LogEvent(
      pLogger, solar::LogLevel::DEBUG, __FILE__, __LINE__, 0,
      solar::GetThreadId(), solar::GetFiberId(), time(0)));
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
  ss << SOLAR_LOG_ROOT()->toYamlString();
  std::string root_log_yaml = R"(name: root
level: DEBUG
formatter: "%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
appenders:
  - type: StdoutLogAppender
    formatter: "%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
  - type: FileLogAppender
    file: test.log
    formatter: "%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n")";
  EXPECT_EQ(ss.str(), root_log_yaml);
  ss.clear();
  ss << solar::LoggerMgr::Instance()->toYamlString();
  std::string logger_mgr_yaml = R"(name: root
level: DEBUG
formatter: "%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
appenders:
  - type: StdoutLogAppender
    formatter: "%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
  - type: FileLogAppender
    file: test.log
    formatter: "%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"logs:
  - name: LoggerWithFileAppender
    level: DEBUG
    formatter: "%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
  - name: XX
    level: DEBUG
    formatter: "%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
  - name: root
    level: DEBUG
    formatter: "%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
    appenders:
      - type: StdoutLogAppender
        formatter: "%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
      - type: FileLogAppender
        file: test.log
        formatter: "%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n")";
  EXPECT_EQ(ss.str(), logger_mgr_yaml);
}

TEST(TestLog, InitLogSystemWithYAML) {
  YAML::Node node = YAML::LoadFile(TEST_DIR + "log.yml");
  solar::Config::LoadFromYaml(node);
  std::stringstream ss;
  ss << solar::LoggerMgr::Instance()->toYamlString();
  std::string logger_mgr_updated_yaml = R"(logs:
  - name: LoggerWithFileAppender
    level: DEBUG
    formatter: "%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
  - name: XX
    level: DEBUG
    formatter: "%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n"
  - name: root
    level: INFO
    formatter: "%d%T%m%n"
    appenders:
      - type: FileLogAppender
        file: root.txt
        formatter: "%d%T%m%n"
      - type: StdoutLogAppender
        formatter: "%d%T%m%n"
  - name: system
    level: DEBUG
    formatter: "%d%T%m%n"
    appenders:
      - type: FileLogAppender
        file: system.txt
        formatter: "%d%T%m%n"
      - type: StdoutLogAppender
        formatter: "%d%T%m%n")";
  EXPECT_EQ(ss.str(), logger_mgr_updated_yaml);
}

#endif