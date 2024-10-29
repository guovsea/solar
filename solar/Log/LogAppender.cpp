#include "Log/LogAppender.h"
#include "LogAppender.h"

namespace solar {

void LogAppender::setFormatter(LogFormatter::ptr formater) {
    m_formatter = formater;
}

LogFormatter::ptr LogAppender::getFormatter() { return m_formatter; }

void LogAppender::setLevel(LogLevel level) { m_level = level; }

LogLevel LogAppender::getLevel() { return m_level; }

} // namespace solar
