#ifndef __SOLAR_UTIL_LEXICALCAST__
#define __SOLAR_UTIL_LEXICALCAST__
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace solar {
template <class F, class T> class LexicalCast {
public:
  T operator()(const F &v) { return boost::lexical_cast<T>(v); }
};

// 偏特化
// std::vector
/**
 * @brief 将 yaml 格式的字符串转成 std::vector<T>
 *
 * @tparam T
 */
template <class T>
// F = std::string, T = std::vector<T>
class LexicalCast<std::string, std::vector<T>> {
public:
  std::vector<T> operator()(const std::string &v) {
    YAML::Node node = YAML::Load(v);
    typename std::vector<T> vec;
    std::stringstream ss;
    for (size_t i = 0; i < node.size(); ++i) {
      ss.str(""); // 清空
      ss << node[i];
      vec.push_back(LexicalCast<std::string, T>{}(ss.str()));
    }
    return vec;
  }
};
/**
 * @brief 将 std::vector<T> 转成 yaml 格式的字符串
 *
 * @tparam T
 */
template <class T>
// F = std::string, T = std::vector<T>
class LexicalCast<std::vector<T>, std::string> {
public:
  std::string operator()(const std::vector<T> &v) {
    YAML::Node node;
    for (const auto &it : v) {
      node.push_back(YAML::Load(LexicalCast<T, std::string>{}(it)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

// std::list
template <class T> class LexicalCast<std::string, std::list<T>> {
public:
  std::list<T> operator()(const std::string &v) {
    YAML::Node node = YAML::Load(v);
    typename std::list<T> vec;
    std::stringstream ss;
    for (size_t i = 0; i < node.size(); ++i) {
      ss.str(""); // 清空
      ss << node[i];
      vec.push_back(LexicalCast<std::string, T>{}(ss.str()));
    }
    return vec;
  }
};

template <class T> class LexicalCast<std::list<T>, std::string> {
public:
  std::string operator()(const std::list<T> &v) {
    YAML::Node node;
    for (const auto &it : v) {
      node.push_back(YAML::Load(LexicalCast<T, std::string>{}(it)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

// std::set
template <class T> class LexicalCast<std::string, std::set<T>> {
public:
  std::set<T> operator()(const std::string &v) {
    YAML::Node node = YAML::Load(v);
    typename std::set<T> set;
    std::stringstream ss;
    for (size_t i = 0; i < node.size(); ++i) {
      ss.str(""); // 清空
      ss << node[i];
      set.insert(LexicalCast<std::string, T>{}(ss.str()));
    }
    return set;
  }
};

template <class T> class LexicalCast<std::set<T>, std::string> {
public:
  std::string operator()(const std::set<T> &v) {
    YAML::Node node;
    for (const auto &it : v) {
      node.push_back(YAML::Load(LexicalCast<T, std::string>{}(it)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

// std::map
template <class T> class LexicalCast<std::string, std::map<std::string, T>> {
public:
  std::map<std::string, T> operator()(const std::string &v) {
    YAML::Node node = YAML::Load(v);
    std::map<std::string, T> map;
    std::stringstream ss;
    for (auto it = node.begin(); it != node.end(); ++it) {
      ss.str(""); // 清空
      ss << it->second;
      map.insert(std::make_pair(it->first.Scalar(),
                                LexicalCast<std::string, T>{}(ss.str())));
    }
    return map;
  }
};

template <class T> class LexicalCast<std::map<std::string, T>, std::string> {
public:
  std::string operator()(const std::map<std::string, T> &v) {
    YAML::Node node;
    for (const auto [key, val] : v) {
      node[key] = YAML::Load(LexicalCast<T, std::string>{}(val));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

// std::unordered_set
template <class T> class LexicalCast<std::string, std::unordered_set<T>> {
public:
  std::unordered_set<T> operator()(const std::string &v) {
    YAML::Node node = YAML::Load(v);
    std::unordered_set<T> set;
    std::stringstream ss;
    for (size_t i = 0; i < node.size(); ++i) {
      ss.str(""); // 清空
      ss << node[i];
      set.insert(LexicalCast<std::string, T>{}(ss.str()));
    }
    return set;
  }
};

template <class T> class LexicalCast<std::unordered_set<T>, std::string> {
public:
  std::string operator()(const std::unordered_set<T> &v) {
    YAML::Node node;
    for (const auto &it : v) {
      node.push_back(YAML::Load(LexicalCast<T, std::string>{}(it)));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};

// std::unordered_map
template <class T>
class LexicalCast<std::string, std::unordered_map<std::string, T>> {
public:
  std::unordered_map<std::string, T> operator()(const std::string &v) {
    YAML::Node node = YAML::Load(v);
    std::unordered_map<std::string, T> map;
    std::stringstream ss;
    for (auto it = node.begin(); it != node.end(); ++it) {
      ss.str(""); // 清空
      ss << it->second;
      map.insert(std::make_pair(it->first.Scalar(),
                                LexicalCast<std::string, T>{}(ss.str())));
    }
    return map;
  }
};

template <class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
public:
  std::string operator()(const std::unordered_map<std::string, T> &v) {
    YAML::Node node;
    for (const auto &it : v) {
      node[it.first] = YAML::Load(LexicalCast<T, std::string>{}(it.second));
    }
    std::stringstream ss;
    ss << node;
    return ss.str();
  }
};
} // namespace solar
#endif