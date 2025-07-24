#include "LogLevel.h"

const char *solar::ToString(LogLevel level) {

    switch (level) {
#define XX(name)                                                                                                       \
    case LogLevel::name:                                                                                               \
        return #name;                                                                                                  \
        break;
        XX(DEBUG)
        XX(INFO)
        XX(WARN)
        XX(ERROR)
        XX(FATAL)
#undef XX
        default:
            return "UNKNOWN";
            break;
    }
    return "UNKNOWN";
}

solar::LogLevel solar::FromString(const std::string &str) {
#define XX(level, v)                                                                                                   \
    if (str == #v) {                                                                                                   \
        return LogLevel::level;                                                                                        \
    }
    XX(DEBUG, debug);
    XX(INFO, info);
    XX(WARN, warn);
    XX(ERROR, error);
    XX(FATAL, fatal);

    XX(DEBUG, DEBUG);
    XX(INFO, INFO);
    XX(WARN, WARN);
    XX(ERROR, ERROR);
    XX(FATAL, FATAL);
    return LogLevel::UNKNOWN;
#undef XX
}