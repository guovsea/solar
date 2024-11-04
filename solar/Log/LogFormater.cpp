#include "Log/LogFormater.h"
#include "Log/Logger.h"

namespace solar {

LogFormatter::LogFormatter(const std::string &pattern) : m_pattern(pattern) {}

std::string LogFormatter::format(std::shared_ptr<Logger> logger, LogLevel level,
                                 LogEvent::ptr event) {
    std::stringstream ss;
    for (auto it : m_items) {
        it->format(ss, logger, level, event);
    }
    return ss.str();
}

class MessageFormatItem : public LogFormatter::FormatItem {
    MessageFormatItem(const std::string &str = "") {}
    void format(std::ostream &os, std::shared_ptr<Logger> logger,
                LogLevel level, LogEvent::ptr event) override {
        os << event->getContent();
    }
};

class LogNameFormatItem : public LogFormatter::FormatItem {
    void format(std::ostream &os, std::shared_ptr<Logger> logger,
                LogLevel level, LogEvent::ptr event) override {
        os << logger->getName();
    }
};

class LevelFormatItem : public LogFormatter::FormatItem {
    void format(std::ostream &os, std::shared_ptr<Logger> logger,
                LogLevel level, LogEvent::ptr event) override {
        os << ToString(level);
    }
};

class FileNameFormatItem : public LogFormatter::FormatItem {
    void format(std::ostream &os, std::shared_ptr<Logger> logger,
                LogLevel level, LogEvent::ptr event) override {
                    os << event->getFile();
                }
};

class LineFormatItem : public LogFormatter::FormatItem {
    void format(std::ostream &, std::shared_ptr<Logger> logger, LogLevel level,
                LogEvent::ptr event) override {}
};

class ThreadIdFormatItem : public LogFormatter::FormatItem {
    void format(std::ostream &, std::shared_ptr<Logger> logger, LogLevel level,
                LogEvent::ptr event) override {}
};

class FiberIdFormatItem : public LogFormatter::FormatItem {
    void format(std::ostream &, std::shared_ptr<Logger> logger, LogLevel level,
                LogEvent::ptr event) override {}
};
class LogNameFormatItem : public LogFormatter::FormatItem {
    void format(std::ostream &, std::shared_ptr<Logger> logger, LogLevel level,
                LogEvent::ptr event) override {}
};

class LineFormatItem : public LogFormatter::FormatItem {
    void format(std::ostream &, std::shared_ptr<Logger> logger, LogLevel level,
                LogEvent::ptr event) override {}
};

class TabFormatItem : public LogFormatter::FormatItem {
    void format(std::ostream &, std::shared_ptr<Logger> logger, LogLevel level,
                LogEvent::ptr event) override {}
};
class NewLineFormatItem : public LogFormatter::FormatItem {
    void format(std::ostream &, std::shared_ptr<Logger> logger, LogLevel level,
                LogEvent::ptr event) override {}
};
} // namespace solar