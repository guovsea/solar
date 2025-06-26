#ifndef __SOLAR_LOG_LOG_H
#define __SOLAR_LOG_LOG_H

#include "Log/LogEvent.h"
#include "Log/Logger.h"
#include "Log/LoggerManager.h"
#include "Util/Util.h"

// 定义一个 LogEventWrap 局部对象，该对象析构时调用 log 方法输出 logEvent 的信息
#define SOLAR_LOG_LEVEL(logger, level)                                         \
  if (logger->getLevel() <= level)                                             \
  solar::LogEventWrap(                                                         \
      solar::LogEvent::ptr(new solar::LogEvent(                                \
          logger, level, __FILE__, __LINE__, 0, solar::GetThreadId(),          \
          solar::GetFiberId(), time(0))))                                      \
      .getSS()

// 流式风格
#define SOLAR_LOG_DEBUG(logger) SOLAR_LOG_LEVEL(logger, solar::LogLevel::DEBUG)
#define SOLAR_LOG_INFO(logger) SOLAR_LOG_LEVEL(logger, solar::LogLevel::INFO)
#define SOLAR_LOG_WARN(logger) SOLAR_LOG_LEVEL(logger, solar::LogLevel::WARN)
#define SOLAR_LOG_ERROR(logger) SOLAR_LOG_LEVEL(logger, solar::LogLevel::ERROR)
#define SOLAR_LOG_FATAL(logger) SOLAR_LOG_LEVEL(logger, solar::LogLevel::FATAL)

#define SOLAR_LOG_FMT_LEVEL(logger, level, fmt, ...)                           \
  if (logger->getLevel() <= level)                                             \
  solar::LogEventWrap(                                                         \
      solar::LogEvent::ptr(new solar::LogEvent(                                \
          logger, level, __FILE__, __LINE__, 0, solar::GetThreadId(),          \
          solar::GetFiberId(), time(0))))                                      \
      .getEvent()                                                              \
      ->format(fmt, __VA_ARGS__)

// prinf 风格
#define SOLAR_LOG_FMT_DEBUG(logger, fmt, ...)                                  \
  SOLAR_LOG_FMT_LEVEL(logger, solar::LogLevel::DEBUG, fmt, __VA_ARGS__)
#define SOLAR_LOG_FMT_INFO(logger, fmt, ...)                                   \
  SOLAR_LOG_FMT_LEVEL(logger, solar::LogLevel::INFO, fmt, __VA_ARGS__)
#define SOLAR_LOG_FMT_WARN(logger, fmt, ...)                                   \
  SOLAR_LOG_FMT_LEVEL(logger, solar::LogLevel::WARN, fmt, __VA_ARGS__)
#define SOLAR_LOG_FMT_ERROR(logger, fmt, ...)                                  \
  SOLAR_LOG_FMT_LEVEL(logger, solar::LogLevel::ERROR, fmt, __VA_ARGS__)
#define SOLAR_LOG_FMT_FATAL(logger, fmt, ...)                                  \
  SOLAR_LOG_FMT_LEVEL(logger, solar::LogLevel::FATAL, fmt, __VA_ARGS__)

// root log
#define SOLAR_LOG_ROOT() solar::LoggerMgr::Instance()->getRoot()
#define SOLAR_LOG_NAME(name) solar::LoggerMgr::Instance()->getLogger(name)

namespace solar {
struct LogIniter {
  LogIniter();
};
} // namespace solar

#endif // !__SOLAR_LOG_LOG_H