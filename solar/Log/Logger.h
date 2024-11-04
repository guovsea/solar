#ifndef SOLAR_LOG_LOGGER_H
#define SOLAR_LOG_LOGGER_H

#include <fstream>
#include <list>
#include <memory>
#include <sstream>
#include <string>

#include "Log/LogAppender.h"
#include "Log/LogEvent.h"
#include "Log/LogFormater.h"

namespace solar {

class Logger : public std::enable_shared_from_this<Logger> {
  public:
    typedef std::shared_ptr<Logger> ptr;
    Logger(const std::string &name = "root");

    void log(LogLevel level, LogEvent::ptr event);
    void debug(LogEvent::ptr event);
    void info(LogEvent::ptr event);
    void warn(LogEvent::ptr event);
    void error(LogEvent::ptr event);
    void fatal(LogEvent::ptr event);

    void addAppender(LogAppender::ptr appender);

    void delAppender(LogAppender::ptr appender);

    void cleanAppenders();

    LogLevel getLoglevel();

    void setLoglevel(LogLevel level);

    std::string getName() const {return m_name;}

  private:
    std::string m_name;
    LogLevel m_level;
    std::list<LogAppender::ptr> m_appenders;
    LogFormatter::ptr m_formater;
};

} // namespace solar

#endif // SOLAR_LOG_LOGGER_H