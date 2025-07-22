#include <iostream>

#include "Log/LogAppender.h"
#include "Log/LogEvent.h"
#include "LogAppender.h"
#include <yaml-cpp/yaml.h>

namespace solar {

void StdoutLogAppender::log(LogLevel level, LogEvent::ptr event) {
  if (level >= m_level) {
    MutexType::Lock lock(m_mutex);
    std::cout << m_formatter->format(level, event);
  }
}

std::string StdoutLogAppender::toYamlString() {
  MutexType::Lock lock(m_mutex);
  YAML::Node node;
  node["type"] = "StdoutLogAppender";
  if (m_level != LogLevel::UNKNOWN) {
    node["level"] = ToString(m_level);
  }
  if (m_formatter) {
    node["formatter"] = m_formatter->getPattern();
  }
  std::stringstream ss;
  ss << node;
  return ss.str();
}

FileLogAppender::FileLogAppender(const std::string &filename)
    : m_filename(filename) {
  reopen();
}

bool FileLogAppender::reopen() {
  if (m_filestream) {
    m_filestream.close();
  }
  m_filestream.open(m_filename, std::ios::app);
  return !!m_filestream; //< !! 惯用法，用来将任意类型的表达式显式转换为 bool
                         // 类型
}

void FileLogAppender::log(LogLevel level, LogEvent::ptr event) {
  if (level >= m_level) {
    uint64_t now = time(0);
    // 更新文件的最近修改时间
    if (now != m_lastTime) {
      reopen();
      m_lastTime = now;
    }
    MutexType::Lock lock(m_mutex);
    m_filestream << m_formatter->format(level, event);
  }
}

std::string FileLogAppender::toYamlString() {
  MutexType::Lock lock(m_mutex);
  YAML::Node node;
  node["type"] = "FileLogAppender";
  node["file"] = m_filename;
  if (m_level != LogLevel::UNKNOWN) {
    node["level"] = ToString(m_level);
  }
  if (m_formatter) {
    node["formatter"] = m_formatter->getPattern();
  }
  std::stringstream ss;
  ss << node;
  return ss.str();
}

void LogAppender::setFormatter(LogFormatter::ptr formater) {
  MutexType::Lock lock(m_mutex);
  m_formatter = formater;
}

LogFormatter::ptr LogAppender::getFormatter() {
  MutexType::Lock lock(m_mutex);
  return m_formatter;
}

} // namespace solar