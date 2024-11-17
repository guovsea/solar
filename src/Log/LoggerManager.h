#ifndef __SOLAR_LOG_LOGGERMANAGER_H__
#define __SOLAR_LOG_LOGGERMANAGER_H__

#include <map>

#include "Util/Singleton.h"
#include "Log/Logger.h"

namespace solar
{
   class LoggerManager
   {
   public:
    LoggerManager();

    Logger::ptr getLogger(const std::string &name);

    void init();
    
    Logger::ptr getRoot() const {return m_root;}
   
   private:
   std::map<std::string, Logger::ptr> m_loggers;
    Logger::ptr m_root;
   };
    
   using LoggerMgr = Singleton<LoggerManager>;
} // namespace solar


#endif /* __SOLAR_LOG_LOGGERMANAGER_H__ */
