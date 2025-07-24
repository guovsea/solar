#ifndef __SOLAR_LOG_LOGEVENT_H__
#define __SOLAR_LOG_LOGEVENT_H__
#include <fstream>
#include <memory>
#include <sstream>
#include <stdint.h>
#include <string>

#include "Log/LogLevel.h"

namespace solar {
class Logger;
class LogEvent {
public:
    typedef std::shared_ptr<LogEvent> ptr;

    LogEvent(std::shared_ptr<Logger> logger, LogLevel level, const char *file, int32_t line, uint32_t elapse,
             uint32_t threadId, uint32_t fiber_id, uint64_t time, const std::string &threadName);
    const char *getFile() const { return m_file; }

    int32_t getLine() const { return m_line; }

    uint32_t getElapse() const { return m_elapse; }

    uint32_t getThreadId() const { return m_threadId; }

    uint32_t getFiberId() const { return m_fiberId; }

    uint64_t getTime() const { return m_time; }

    std::string getContent() const { return m_ss.str(); }

    std::shared_ptr<Logger> getLogger() const { return m_pLogger; }

    LogLevel getLevel() const { return m_level; }

    std::stringstream &getSS() { return m_ss; }

    const std::string &getThreadName() const { return m_threadName; }
    /**
     * @brief 传入 logEvent  的内容
     *
     * @param fmt
     * @param ...
     */
    void format(const char *fmt, ...);

    void format(const char *fmt, va_list al);

private:
    const char *m_file = nullptr;
    int32_t m_line = 0;
    uint32_t m_elapse = 0;
    uint32_t m_threadId = 0;
    uint32_t m_fiberId = 0;
    uint64_t m_time = 0;
    std::string m_threadName;
    std::stringstream m_ss;

    std::shared_ptr<Logger> m_pLogger;
    LogLevel m_level;
};

class LogEventWrap {
public:
    LogEventWrap(LogEvent::ptr e) : m_event(e) {}
    ~LogEventWrap();

    LogEvent::ptr getEvent() const { return m_event; }

    std::stringstream &getSS() { return m_event->getSS(); }

private:
    LogEvent::ptr m_event;
};

} // namespace solar

#endif // !__SOLAR_LOG_LOGEVENT_H