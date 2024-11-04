#include "Logger.h"

namespace solar {

Logger::Logger(const std::string &name)
    : m_name(name), m_level(LogLevel::DEBUG),
      m_formater(new LogFormatter(
          "%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n")) {}

void Logger::log(LogLevel level, LogEvent::ptr event) {
  for (auto appender : m_appenders) {

    auto self = shared_from_this();
    appender->log(self, level, event);
  }
}

void Logger::debug(LogEvent::ptr event) { log(LogLevel::DEBUG, event); }

void Logger::info(LogEvent::ptr event) { log(LogLevel::INFO, event); }

void Logger::warn(LogEvent::ptr event) { log(LogLevel::WARN, event); }

void Logger::error(LogEvent::ptr event) { log(LogLevel::ERROR, event); }

void Logger::fatal(LogEvent::ptr event) { log(LogLevel::FATAL, event); }

void Logger::addAppender(LogAppender::ptr appender) {
  m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
    for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it) {
        if (*it == appender) {
            m_appenders.erase(it);
            break;
        }
    }

}

void Logger::cleanAppenders() {
    m_appenders.clear();
}

LogLevel Logger::getLoglevel() { 
    return m_level;
    }

void Logger::setLoglevel(LogLevel level)
{
    m_level = level;
}

} // namespace solar