#include <yaml-cpp/yaml.h>

#include "LoggerManager.h"

namespace solar {
LoggerManager::LoggerManager() {
    m_root.reset(new Logger);
    m_root->addAppender(std::make_shared<StdoutLogAppender>());
    m_loggers[m_root->m_name] = m_root;
    init();
}
Logger::ptr LoggerManager::getLogger(const std::string &name) {
    MutexType::Lock lock(m_mutex);
    auto it = m_loggers.find(name);
    if (it != m_loggers.end()) {
        return it->second;
    }
    Logger::ptr logger = std::make_shared<Logger>(name);
    logger->m_root = m_root;
    m_loggers[name] = logger;
    return logger;
}
void LoggerManager::init() {}
std::string LoggerManager::toYamlString() {
    MutexType::Lock lock(m_mutex);
    YAML::Node node;
    for (const auto &[key, val]: m_loggers) {
        node["logs"].push_back(YAML::Load(val->toYamlString()));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
}
} // namespace solar