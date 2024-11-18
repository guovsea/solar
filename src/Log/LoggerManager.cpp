#include "LoggerManager.h"

namespace solar{LoggerManager::LoggerManager() {
    m_root.reset(new Logger);
    m_root->addAppender(std::make_shared<StdoutLogAppender>());
    m_loggers[m_root->m_name] = m_root;
    init();
}
Logger::ptr LoggerManager::getLogger(const std::string &name) {
    auto it = m_loggers.find(name);
    if (it != m_loggers.end()) {
        return it->second;
    }
    Logger::ptr logger= std::make_shared<Logger>(name);
    logger->m_root = m_root;
    return logger;
}
void LoggerManager::init() {}
} // namespace solar