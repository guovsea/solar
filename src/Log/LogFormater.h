#ifndef __SOLAR_LOG_LOGFORMATTER_H__
#define __SOLAR_LOG_LOGFORMATTER_H__

#include <memory>
#include <string>
#include <vector>

#include "Log/LogEvent.h"
#include "Log/LogLevel.h"

namespace solar {
class Logger;
class LogFormatter {
public:
    typedef std::shared_ptr<LogFormatter> ptr;
    LogFormatter(const std::string &pattern);

    /**
     * @brief 将 log 信息输出为 string
     *
     * @param level
     * @param event
     * @return std::string
     */
    std::string format(LogLevel level, LogEvent::ptr event);

    void init();

    bool isError() { return m_error; }

    std::string getPattern() const { return m_pattern; }
    class FormatItem {
    public:
        typedef std::shared_ptr<FormatItem> ptr;
        virtual ~FormatItem() {}
        virtual void format(std::ostream &, LogLevel level, LogEvent::ptr event) = 0;
    };

private:
    std::string m_pattern;
    std::vector<FormatItem::ptr> m_items;
    bool m_error = false;
};
} // namespace solar

#endif // !__SOLAR_LOG_LOGFORMATTER_H