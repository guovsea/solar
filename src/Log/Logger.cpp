#include <iostream>
#include <yaml-cpp/yaml.h>

#include "Logger.h"

namespace solar {

Logger::Logger(const std::string &name)
    : m_name(name), m_level(LogLevel::DEBUG),
      m_formater(new LogFormatter(
          "%d{%Y-%m-%d %H:%M:%S}%T%t%T%F%T[%p]%T[%c]%T%f:%l%T%m%n")) {}

void Logger::log(LogLevel level, LogEvent::ptr event) {
  MutexType::ScopedLock lock(m_mutex);
  if (level >= m_level) {
    auto self = shared_from_this();
    if (!m_appenders.empty()) {
      for (auto i : m_appenders) {
        i->log(level, event);
      }
      // 如果新创建了 logger 没有设置 appender ，
      // 使用 loger Mgr 中的 root logger 的 log appenders
    } else if (m_root) {
      m_root->log(level, event);
    }
  }
}

void Logger::debug(LogEvent::ptr event) { log(LogLevel::DEBUG, event); }

void Logger::info(LogEvent::ptr event) { log(LogLevel::INFO, event); }

void Logger::warn(LogEvent::ptr event) { log(LogLevel::WARN, event); }

void Logger::error(LogEvent::ptr event) { log(LogLevel::ERROR, event); }

void Logger::fatal(LogEvent::ptr event) { log(LogLevel::FATAL, event); }

void Logger::addAppender(LogAppender::ptr appender) {
  MutexType::ScopedLock lock(m_mutex);
  if (!appender->getFormatter()) {
    appender->setFormatter(m_formater);
  }
  m_appenders.push_back(appender);
}

void Logger::delAppender(LogAppender::ptr appender) {
  MutexType::ScopedLock lock(m_mutex);
  for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it) {
    if (*it == appender) {
      m_appenders.erase(it);
      break;
    }
  }
}

void Logger::cleanAppenders() {
  MutexType::ScopedLock lock(m_mutex);
  m_appenders.clear();
}

void Logger::setFormatter(LogFormatter::ptr val) {
  MutexType::ScopedLock lock(m_mutex);
  m_formater = val;
}

void Logger::setFormatter(const std::string &val) {
  MutexType::ScopedLock lock(m_mutex);
  LogFormatter::ptr new_val(new LogFormatter(val));
  if (new_val->isError()) {
    std::cout << "Logger setFormatter name=" << m_name << " value=" << val
              << " invalid formatter" << std::endl;
    return;
  }
  m_formater = new_val;
}

LogLevel Logger::getLevel() {
  MutexType::ScopedLock lock(m_mutex);
  return m_level;
}

void Logger::setLevel(LogLevel level) {
  MutexType::ScopedLock lock(m_mutex);
  m_level = level;
}

std::string Logger::toYamlString() {
  MutexType::ScopedLock lock(m_mutex);
  YAML::Node node;
  node["name"] = m_name;
  node["level"] = ToString(m_level);
  if (m_formater) {
    node["formatter"] = m_formater->getPattern();
  }
  for (const auto &it : m_appenders) {
    node["appenders"].push_back(YAML::Load(it->toYamlString()));
  }
  std::stringstream ss;
  ss.clear();
  ss << node;
  return ss.str();
}

} // namespace solar