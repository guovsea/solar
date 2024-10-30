#include "LogFormater.h"

solar::LogFormatter::LogFormatter(const std::string &pattern)
    : m_pattern(pattern) {}

std::string solar::LogFormatter::format(std::shared_ptr<Logger> logger,
                                        LogLevel level, LogEvent::ptr event) {
    std::stringstream ss;
    for (auto it : m_items) {
        it->format(ss, logger, level, event);
    }
    return ss.str();
}
