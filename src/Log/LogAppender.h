#ifndef __SOLAR_LOG_LOGAPPENDER_H__
#define __SOLAR_LOG_LOGAPPENDER_H__

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
    virtual void log(LogLevel level,
                     LogEvent::ptr event) = 0;

    virtual std::string toYamlString() const = 0;

    void setFormatter(LogFormatter::ptr formater) ;

    LogFormatter::ptr getFormatter() const;

    void setLevel(LogLevel level);

    LogLevel getLevel() const;

  protected:
    LogLevel m_level;
    LogFormatter ::ptr m_formatter;
};

class StdoutLogAppender :public LogAppender {
  public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;

    void log(LogLevel level,
                     LogEvent::ptr event) override;

    std::string toYamlString() const override;
};
class FileLogAppender :public LogAppender{
  public:
    typedef std::shared_ptr<FileLogAppender> ptr;

    FileLogAppender(const std::string& filename);

    bool reopen();

    void log(LogLevel level,
                     LogEvent::ptr event) override;

    std::string toYamlString() const override;

  private:
    std::string m_filename;
    std::ofstream m_filestream;
};
} // namespace solar

#endif // !__SOLAR_LOG_LOGAPPENDER_H