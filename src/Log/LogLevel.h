#ifndef __SOLAR_LOG_LOGLEVEL_H__
#define __SOLAR_LOG_LOGLEVEL_H__
#include <string>

namespace solar {
enum class LogLevel {
    UNKNOWN = 0,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    DELETED = 1000
};

const char *ToString(LogLevel level);

LogLevel FromString(const std::string &str);
} // namespace solar
#endif // !__SOLAR_LOG_LOGLEVEL_H