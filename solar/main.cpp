#include "Log/Log.h"
#include "Util/Util.h"
#include <iostream>

int main (int argc, char *argv[]) {
  solar::Logger::ptr pLogger(new solar::Logger);
  pLogger->addAppender(solar::LogAppender::ptr(new solar::StdoutLogAppender));

  solar::LogEvent::ptr pEvent(new solar::LogEvent(pLogger, solar::LogLevel::DEBUG, __FILE__, __LINE__, 0, solar::GetThreadId(), solar::GetFiberId(), time(0)));
  pEvent->getSS() << "hello solar log";

  pLogger->log(solar::LogLevel::DEBUG, pEvent);

  SOLAR_LOG_DEBUG(pLogger) << "log debug";
  SOLAR_LOG_INFO(pLogger) << "log info";
  SOLAR_LOG_FMT_DEBUG(pLogger, "%s %s", "log", "debug");

  auto l = LoggerMgr::Instance()->getLogger("XX");
  SOLAR_LOG_DEBUG(l) << "hello loggerMgr";

  return 0;
}
