#ifndef SOLAR_LOG_LOGFORMATTER_H
#define SOLAR_LOG_LOGFORMATTER_H

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
    * @param logger  // 虽然只有 NameFormatItem 需要用到 logger，但为了保持接口的一致性给所有类型的 item 都传入 logger
    * @param level 
    * @param event 
    * @return std::string 
    */
    std::string format(std::shared_ptr<Logger> logger, LogLevel level,
                       LogEvent::ptr event);

  private:
    void init();
    class FormatterItem {
      public:
        typedef std::shared_ptr<FormatterItem> ptr;
        virtual ~FormatterItem() {}
        virtual void format(std::ostream &, std::shared_ptr<Logger> logger,
                            LogLevel level, LogEvent::ptr event) = 0;
    };

    std::string m_pattern;
    std::vector<FormatterItem::ptr> m_items;
};
} // namespace solar

#endif // !SOLAR_LOG_LOGFORMATTER_H