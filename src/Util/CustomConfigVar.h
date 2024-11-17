#ifndef __SOLAR_UTIL_CUSTOMCONFIGVAR_H__
#define __SOLAR_UTIL_CUSTOMCONFIGVAR_H__
#include <string>
#include <yaml-cpp/yaml.h>

namespace solar {

/**
 * @brief
 *
 * @param node
 * @param key
 * @param val 穿出参数
 * @return true
 * @return false
 */
template <class T>
bool FromYaml(const YAML::Node &node, const std::string &key, T &val) {
    if (node[key]) {
        val = node[key].as<T>();
        return true;
    }
    return false;
}

// 递归解析变参模板函数
inline bool FromYaml(const YAML::Node &node) { return true; }

template <typename T, typename... Args>
bool FromYaml(const YAML::Node &node, const std::string &key, T &val,
              Args &&...args) {
    return FromYaml(node, key, val) && FromYaml(node, args...);
}

/**
 * @brief
 *
 * @tparam Args
 * @param str yaml 格式的字符串
 * @param args key, output[value], key, output[value] ...
 * @return true
 * @return false
 */
template <typename... Args>
bool FromYamlString(const std::string &str, Args &&...args) {
    YAML::Node node = YAML::Load(str);
    return FromYaml(node, args...);
}

/**
 * @brief
 *
 * @tparam T
 * @param node
 * @param key
 * @param val
 */
template <class T>
void ToYaml(YAML::Node &node, const std::string &key, const T &val) {
    node[key] = val;
}

inline void ToYaml(YAML::Node &node) {}

template <typename T, typename... Args>
void ToYaml(YAML::Node &node, const std::string &key, const T &val,
            Args &&...args) {
    ToYaml(node, key, val);
    ToYaml(node, args...);
}

/**
 * @brief
 *
 * @tparam Args
 * @param args key, value, key, value ...
 * @return std::string yaml 格式的字符串
 */
template <typename... Args> std::string ToYamlString(Args &&...args) {
    YAML::Node node;
    ToYaml(node, args...);
    std::stringstream ss;
    ss << node;
    return ss.str();
}

struct CustomConfigVar {
  public:
    virtual ~CustomConfigVar() {}
    virtual bool fromYaml(const std::string &str) = 0;
    virtual std::string toYaml() const = 0;
};
} // namespace solar

#define RIGISTER_SOLAR_CONFIG(Type)                                            \
    namespace solar {                                                          \
    template <> class LexicalCast<std::string, Type> {                         \
      public:                                                                  \
        Type operator()(const std::string &str) {                              \
            Type v;                                                            \
            v.fromYaml(str);                                                   \
            return v;                                                          \
        }                                                                      \
    };                                                                         \
    template <> class LexicalCast<Type, std::string> {                         \
      public:                                                                  \
        std::string operator()(const Type &v) { return v.toYaml(); }           \
    };                                                                         \
    }

#endif /* __SOLAR_UTIL_CUSTOMCONFIGVAR_H__ */
