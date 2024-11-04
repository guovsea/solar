#include <ostream>
#include <stdarg.h>

#include "LogEvent.h"

solar::LogEvent::LogEvent(std::shared_ptr<Logger> logger, LogLevel level,
                          const char *file, int32_t line, uint32_t elapse,
                          uint32_t threadId, uint32_t fiberId, uint64_t time)
    : m_file(file), m_line(line), m_elapse(elapse), m_threadId(threadId),
      m_fiberId(fiberId), m_time(time), m_ss(), m_pLogger(logger),
      m_level(level) {}

void solar::LogEvent::format(const char *fmt, ...) {
    va_list al;
    va_start(al, fmt);
    format(fmt, al);
    va_end(al);
}

void solar::LogEvent::format(const char *fmt, va_list al) {
    char *buf = nullptr;
    int len = vasprintf(&buf, fmt, al);
    if (len != -1) {
        m_ss << std::string(buf, len);
        free(buf);
    }
}
