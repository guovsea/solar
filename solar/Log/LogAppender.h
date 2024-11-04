#ifndef SOLAR_LOG_LOGAPPENDER_H
#define SOLAR_LOG_LOGAPPENDER_H

#include <fstream>
#include <memory>
#include <string>

#include "Log/LogEvent.h"
#include "Log/LogFormater.h"

namespace solar {
class Logger;
class LogAppender {
  public:
    typedef std::shared_ptr<LogAppender> ptr;

    virtual ~LogAppender() {}

    // logger 会传给 FormatterItem
    virtual void log(std::shared_ptr<Logger> logger, LogLevel level,
                     LogEvent::ptr event) = 0;

    void setFormatter(LogFormatter::ptr formater) ;
    LogFormatter::ptr getFormatter()const;

    void setLevel(LogLevel level);
    LogLevel getLevel() const;

  protected:
    LogLevel m_level;
    LogFormatter ::ptr m_formatter;
};

class StdoutLogAppender :public LogAppender {
  public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;

    void log(std::shared_ptr<Logger> logger, LogLevel level,
                     LogEvent::ptr event) override;
};
class FileLogAppender :public LogAppender{
  public:
    typedef std::shared_ptr<FileLogAppender> ptr;

    void log(std::shared_ptr<Logger> logger, LogLevel level,
                     LogEvent::ptr event) override;

  private:
    std::string m_filename;
    std::ofstream m_filestream;
};
} // namespace solar

#endif // !SOLAR_LOG_LOGAPPENDER_H