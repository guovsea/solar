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

    virtual void log(std::shared_ptr<LogAppender> logger, LogLevel level,
                     LogEvent::ptr event) = 0;

    void setFormatter(LogFormatter::ptr formater);
    LogFormatter::ptr getFormatter();

    void setLevel(LogLevel level);
    LogLevel getLevel();

  protected:
    LogLevel m_level;
    LogFormatter ::ptr m_formatter;
};

class StdoutLogAppender {

  public:
    typedef std::shared_ptr<StdoutLogAppender> ptr;
};
class FileLogAppender {
  public:
    typedef std::shared_ptr<FileLogAppender> ptr;

  private:
    std::string m_filename;
    std::ofstream m_filestream;
};
} // namespace solar