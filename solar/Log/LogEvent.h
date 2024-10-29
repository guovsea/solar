#include <stdint.h>
#include <memory>
#include <string>
#include <sstream>
#include <fstream>

#include "Log/LogLevel.h"

namespace solar
{
class Logger;
class LogEvent {
  public:
    typedef std::shared_ptr<LogEvent> ptr;
    const char *m_file = nullptr;
    int32_t m_line = 0;
    uint32_t m_elapse = 0;
    uint32_t m_threadId = 0;
    uint32_t m_fiberId = 0;
    uint64_t m_time = 0;
    std::string m_threadName;
    std::stringstream m_ss;

    std::shared_ptr<Logger> m_pLog;
    LogLevel m_level;
};

} // namespace solar