#include <iostream>

#include "Log/LogAppender.h"
#include "LogAppender.h"

namespace solar {

void LogAppender::setFormatter(LogFormatter::ptr formater) {
    m_formatter = formater;
}

LogFormatter::ptr LogAppender::getFormatter() const { return m_formatter; }

void LogAppender::setLevel(LogLevel level) { m_level = level; }

LogLevel LogAppender::getLevel() const { return m_level; }

void StdoutLogAppender::log(std::shared_ptr<Logger> logger, LogLevel level,
                            LogEvent::ptr event) {
    if (level >= m_level)
        std::cout << m_formatter->format(logger, level, event);
}

void FileLogAppender::log(std::shared_ptr<Logger> logger, LogLevel level,
                          LogEvent::ptr event) {
                            // todo

                          }

} // namespace solar
