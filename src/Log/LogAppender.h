#ifndef __SOLAR_LOG_LOGAPPENDER_H__
#define __SOLAR_LOG_LOGAPPENDER_H__

#include <fstream>
#include <memory>
#include <string>

#include "Core/Mutex.h"
#include "Log/LogEvent.h"
#include "Log/LogFormater.h"

namespace solar {
class Logger;
class LogAppender {
public:
    typedef std::shared_ptr<LogAppender> ptr;
    typedef Mutex MutexType;
    virtual ~LogAppender() {}

    // logger 会传给 FormatterItem
    virtual void log(LogLevel level, LogEvent::ptr event) = 0;

    virtual std::string toYamlString() = 0;

    void setFormatter(LogFormatter::ptr formater);

    LogFormatter::ptr getFormatter();

    void setLevel(LogLevel level) { m_level = level; }

    LogLevel getLevel() const { return m_level; }

protected:
    LogLevel m_level;
    LogFormatter::ptr m_formatter = nullptr;
    MutexType m_mutex;
};

class StdoutLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;

    void log(LogLevel level, LogEvent::ptr event) override;

    std::string toYamlString() override;
};
class FileLogAppender : public LogAppender {
public:
    typedef std::shared_ptr<FileLogAppender> ptr;

    FileLogAppender(const std::string &filename);

    bool reopen();

    void log(LogLevel level, LogEvent::ptr event) override;

    std::string toYamlString() override;

private:
    std::string m_filename;
    std::ofstream m_filestream;
    uint64_t m_lastTime = 0;
};
} // namespace solar

#endif // !__SOLAR_LOG_LOGAPPENDER_H