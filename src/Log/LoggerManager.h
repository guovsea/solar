#ifndef __SOLAR_LOG_LOGGERMANAGER_H__
#define __SOLAR_LOG_LOGGERMANAGER_H__

#include <map>

#include "Core/Mutex.h"
#include "Log/Logger.h"
#include "Util/Singleton.h"

namespace solar {
class LoggerManager {
public:
    typedef Mutex MutexType;
    LoggerManager();

    Logger::ptr getLogger(const std::string &name);

    void init();

    Logger::ptr getRoot() const { return m_root; }

    std::string toYamlString();

private:
    std::map<std::string, Logger::ptr> m_loggers;
    Logger::ptr m_root;
    MutexType m_mutex;
};

using LoggerMgr = Singleton<LoggerManager>;
} // namespace solar

#endif /* __SOLAR_LOG_LOGGERMANAGER_H__ */
