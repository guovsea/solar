#include <gtest/gtest.h>

#include "Log/Log.h"
#include "Util/Config.h"
#include "Util/Util.h"

// Demonstrate some basic assertions.
TEST(TestLog, TestLog) {
    GTEST_SKIP();
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

TEST(TestLog, LoggerWithOutAppender) {
    SOLAR_LOG_ROOT()->addAppender(
        std::make_shared<solar::FileLogAppender>("test.log"));
    SOLAR_LOG_DEBUG(SOLAR_LOG_NAME("LoggerWithOutAppender")) << "hello world";
}
TEST(TestLog, ToYamlString) {
    std::cout << SOLAR_LOG_ROOT()->toYamlString();
    std::cout << std::endl;
    std::cout << solar::LoggerMgr::Instance()->toYamlString();
    std::cout << std::endl;
}


TEST(TestLog, InitLogSystemWithYAML) {
    solar::Logger::ptr system_log = SOLAR_LOG_NAME("system");
    SOLAR_LOG_INFO(system_log) << "hello system" << std::endl;

    std::cout << "=====before=======" << std::endl;
    std::cout << solar::LoggerMgr::Instance()->toYamlString() << std::endl;
    std::cout << "=====before=======" << std::endl;

    YAML::Node root = YAML::LoadFile("log.yml");
    solar::Config::LoadFromYaml(root);
    std::cout << "======after=======" << std::endl;
    std::cout << solar::LoggerMgr::Instance()->toYamlString() << std::endl;
    std::cout << "======after=======" << std::endl;

    SOLAR_LOG_INFO(system_log) << "hello system" << std::endl;
}