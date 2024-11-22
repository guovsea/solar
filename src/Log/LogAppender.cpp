#include <iostream>

#include "Log/LogAppender.h"
#include "Log/LogEvent.h"
#include "LogAppender.h"
#include <yaml-cpp/yaml.h>

namespace solar {

void LogAppender::setFormatter(LogFormatter::ptr formater) {
    m_formatter = formater;
}

LogFormatter::ptr LogAppender::getFormatter() const { return m_formatter; }

void LogAppender::setLevel(LogLevel level) { m_level = level; }

LogLevel LogAppender::getLevel() const { return m_level; }

void StdoutLogAppender::log(LogLevel level, LogEvent::ptr event) {
    if (level >= m_level)
        std::cout << m_formatter->format(level, event);
}

std::string StdoutLogAppender::toYamlString() const {
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
        m_filestream << m_formatter->format(level, event);
    }
}

std::string FileLogAppender::toYamlString() const {
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


} // namespace solar
