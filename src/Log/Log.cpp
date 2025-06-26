#include <iostream>
#include <yaml-cpp/yaml.h>

#include "Log/Log.h"
#include "Util/Config.h"

namespace solar {

enum class LogAppenderType { Unknown, StdoutLogAppender, FileLogAppender };
struct LogAppenderDefine {
  LogAppenderType type = LogAppenderType::Unknown;
  LogLevel level = LogLevel::UNKNOWN;
  std::string formatter;
  std::string file;
  bool valid = true;
  std::string toYamlString() const;
  static LogAppenderDefine FromYamlString(const std::string &str);
  bool operator==(const LogAppenderDefine &oth) const {
    return type == oth.type && level == oth.level &&
           formatter == oth.formatter && file == oth.file;
  }
};

struct LoggerDefine {
  std::string name;
  LogLevel level = LogLevel::UNKNOWN;
  std::string formatter;
  std::vector<LogAppenderDefine> appenders;
  bool valid = true;
  std::string toYamlString() const;
  static LoggerDefine FromYamlString(const std::string &str);
  bool operator<(const LoggerDefine &oth) const { return name < oth.name; }
  bool operator==(const LoggerDefine &oth) const {
    return name == oth.name && level == oth.level &&
           formatter == oth.formatter && appenders == oth.appenders;
  }
};

template <> class LexicalCast<std::string, std::set<LoggerDefine>> {
public:
  std::set<LoggerDefine> operator()(const std::string &str) {
    std::set<LoggerDefine> res;
    YAML::Node node = YAML::Load(str);
    for (auto x : node) {
      std::string s = YAML::Dump(x);
      auto loggerDef = LoggerDefine::FromYamlString(s);
      if (!loggerDef.valid) {
        continue;
      }
      res.insert(loggerDef);
    }
    return res;
  }
};

template <> class LexicalCast<std::set<LoggerDefine>, std::string> {
public:
  std::string operator()(const std::set<LoggerDefine> &v) {
    std::stringstream ss;
    YAML::Node node;
    for (const auto &it : v) {
      node["logs"].push_back(YAML::Load(it.toYamlString()));
    }
    return ss.str();
  }
};

static solar::ConfigVar<std::set<LoggerDefine>>::ptr g_logger_defines =
    solar::Config::Lookup("logs", std::set<LoggerDefine>{}, "logs config");

/**
 * @brief 在 main 函数执行之前给 logs config 设置毁掉函数
 * 静态初始化器，可以在 main 函数之前执行
 *
 */
LogIniter::LogIniter() {
  auto updateLogSystem = [](const std::set<LoggerDefine> &oldVal,
                            const std::set<LoggerDefine> &newVal) {
    SOLAR_LOG_INFO(SOLAR_LOG_ROOT()) << "on_logger_config_changed";
    for (const auto &x : newVal) {
      assert(x.valid);
      auto it = oldVal.find(x);
      // 学习此种方法：1. 找到需要更新的 logger，无论是新增的还是修改的
      // LoggerDefine 重载的 == 是所有数据必须完全一样，所以当 it != end
      // 时，新的 x 必定和旧的 *it 完全一摸一样
      solar::Logger::ptr pLogger;
      if (it == oldVal.end()) {
        // 新增的 loggerDefine
        pLogger = SOLAR_LOG_NAME(x.name);
      } else {
        // 修改的 loggerDefine
        if (!(x == *it)) {
          pLogger = SOLAR_LOG_NAME(x.name);
        }
      }
      // 2. 只需要对需要更新的 logger 更新
      pLogger->setLevel(x.level);
      if (!x.formatter.empty()) {
        pLogger->setFormatter(x.formatter);
      }
      pLogger->cleanAppenders(); //< 记得先清空
      for (const auto &x : x.appenders) {
        solar::LogAppender::ptr pAppender;
        if (x.type == LogAppenderType::FileLogAppender) {
          pAppender = std::make_shared<FileLogAppender>(x.file);
        } else if (x.type == LogAppenderType::StdoutLogAppender) {
          pAppender = std::make_shared<StdoutLogAppender>();
        }
        pAppender->setLevel(x.level);
        pLogger->addAppender(pAppender);
      }
      for (auto &x : oldVal) {
        auto it = newVal.find(x);
        if (it == newVal.end()) {
          // 删除不存在的 logger
          auto logger = SOLAR_LOG_NAME(it->name);
          logger->setLevel(solar::LogLevel::DELETED);
          logger->cleanAppenders();
        }
      }
    }
  };
  // 随便定的数字
  g_logger_defines->addListener(0x9527, updateLogSystem);
}

// TODO 不知道为什么，在 log 模块中的任何 cpp 文件中定义的静态变量都不会被初始化
// 除非定义在被直接编译进可执行文件的 cpp 文件中
static LogIniter __logInit;

std::string LogAppenderDefine::toYamlString() const {
  YAML::Node node;
  if (type == LogAppenderType::FileLogAppender) {
    node["type"] = "FileLogAppender";
  }
  if (type == LogAppenderType::StdoutLogAppender) {
    node["type"] = "StdoutLogAppender";
  }
  if (level != LogLevel::UNKNOWN) {
    node["level"] = ToString(level);
  }
  if (!formatter.empty()) {
    node["formatter"] = formatter;
  }
  if (!file.empty()) {
    node["file"] = file;
  }
  std::stringstream ss;
  ss << node;
  return ss.str();
}
LogAppenderDefine LogAppenderDefine::FromYamlString(const std::string &str) {
  LogAppenderDefine res;
  const YAML::Node node = YAML::Load(str);
  bool valid = true;
  if (node["type"].IsDefined()) {
    std::string typeStr = node["type"].as<std::string>();
    if (typeStr == "FileLogAppender") {
      res.type = LogAppenderType::FileLogAppender;
    } else if (typeStr == "StdoutLogAppender") {
      res.type = LogAppenderType::StdoutLogAppender;
    } else {
      valid = false;
      std::cout << "log config error: log appender type is valid, " << node
                << std::endl;
    }
  } else {
    valid = false;
    std::cout << "log config error: log appender type is valid, " << node
              << std::endl;
  }
  if (node["level"].IsDefined()) {
    res.level = FromString(node["level"].as<std::string>());
  }
  if (node["file"].IsDefined()) {
    res.file = node["file"].as<std::string>();
  } else {
    if (res.type == LogAppenderType::FileLogAppender) {
      std::cout << "log config error: fileappender file is null, " << node
                << std::endl;
      valid = false;
    }
  }
  res.valid = valid;
  return res;
}
std::string LoggerDefine::toYamlString() const {
  YAML::Node node;
  node["name"] = name;
  if (level != LogLevel::UNKNOWN) {
    node["level"] = ToString(level);
  }
  if (!formatter.empty()) {
    node["formatter"] = formatter;
  }
  for (const auto &it : appenders) {
    node["appenders"].push_back(YAML::Load(it.toYamlString()));
  }
  std::stringstream ss;
  ss << node;
  return ss.str();
}
LoggerDefine LoggerDefine::FromYamlString(const std::string &str) {
  LoggerDefine res;
  YAML::Node node = YAML::Load(str);
  bool valid = true;
  if (node["name"].IsDefined()) {
    res.name = node["name"].as<std::string>();
  } else {
    std::cout << "log config error: logger name is null, " << node << std::endl;
    valid = false;
  }
  if (node["level"].IsDefined()) {
    res.level = FromString(node["level"].as<std::string>());
  }
  if (node["formatter"].IsDefined()) {
    res.formatter = node["formatter"].as<std::string>();
  }
  if (node["appenders"].IsSequence()) {
    for (auto it = node["appenders"].begin(); it != node["appenders"].end();
         ++it) {
      std::string s = YAML::Dump(*it);
      LogAppenderDefine appenderDef = LogAppenderDefine::FromYamlString(s);
      if (!appenderDef.valid) {
        valid = false;
      }
      res.appenders.push_back(appenderDef);
    }
  }
  res.valid = valid;
  return res;
}

} // namespace solar