#ifndef __SOLAR_UTIL_CONFIG_H__
#define __SOLAR_UTIL_CONFIG_H__
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <yaml-cpp/yaml.h>

#include "Log/Log.h"

namespace solar {
class ConfigVarBase {
  public:
    typedef std::shared_ptr<ConfigVarBase> ptr;
    ConfigVarBase(const std::string &name, const std::string &discription = "")
        : m_name(name), m_discription(discription) {
        // effective C++ item:25
        using std::tolower;
        std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
    }
    virtual ~ConfigVarBase() {}

    std::string getName() const { return m_name; }

    void setName(const std::string &name) { m_name = name; }

    std::string getDiscription() const { return m_discription; }

    void setDiscription(const std::string &discription) {
        m_discription = discription;
    }
    /**
     * @brief 从 yaml 格式的 string 中 获取 config 的 value
     *
     * @param str yaml 格式的 string， 在 yaml 中对应的 yaml 节点的 string
     * @return true
     * @return false
     */
    virtual bool fromYaml(const std::string &str) = 0;

    virtual std::string toYaml() const = 0;

    virtual std::string getTypeName() const = 0;

  protected:
    std::string m_name;
    std::string m_discription;
};

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
        typename std::map<std::string, T> map;
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
        typename std::unordered_set<T> set;
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
        typename std::unordered_map<std::string, T> map;
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
            node[it.first] =
                YAML::Load(LexicalCast<T, std::string>{}(it.second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

/**
 * @brief 
 * 
 * @tparam T 值的类型
 * @tparam FromStr yaml 格式的字符串转到 T 使用的可调用对象的类型 
 * @tparam T> 
 * @tparam ToStr T 转到 yaml 格式的字符串使用的的可调用对象的类型  
 * @tparam std::string> 
 */
template <class T, class FromStr = LexicalCast<std::string, T>,
          class ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase {
  public:
    typedef std::shared_ptr<ConfigVar> ptr;
    typedef std::function<void(const T &oldVal, const T &newVal)>
        changedCallBack;
    ConfigVar(const std::string &name, const T &defaultValue,
              const std::string &description = "")
        : ConfigVarBase(name, description), m_value(defaultValue) {}
    bool fromYaml(const std::string &str) override {
        try {
            setValue(FromStr{}(str));
            return true;
        } catch (std::exception &e) {
            SOLAR_LOG_ERROR(SOLAR_LOG_ROOT())
                << "ConfigVar::fromYaml exception" << e.what()
                << "convert: string to " << getTypeName()
                // 这样 T 都需要重载 << ，是否值得？
                /*<< " - " << val*/
                ;
            return false;
        }
    }
    std::string toYaml() const override {
        try {
            return ToStr{}(m_value);
        } catch (std::exception &e) {
            SOLAR_LOG_ERROR(SOLAR_LOG_ROOT())
                << "ConfigVar::toYaml exception" << e.what()
                << "convert: " << getTypeName() << " to string";
        }
        return "";
    }
    std::string getTypeName() const override { return typeid(m_value).name(); }

    T getValue() const { return m_value; }

    void setValue(const T &value) { 
        if (value == m_value) {
            return;
        }
        for (auto& [key, val]: m_cbs) {
            val(m_value, value);
        }
        m_value = value; }

    bool operator==(const ConfigVar<T> &other) {
        return m_value == other.m_value;
    }

    /**
     * @brief 值改变时会触发的回调函数
     *
     * @param key 必须唯一
     * @param cb
     */
    void addListener(uint64_t key, changedCallBack cb) { m_cbs[key] = cb; }

    changedCallBack delListener(uint64_t key) {
        auto it = m_cbs.find(key);
        return it == m_cbs.end() ? nullptr : it->second;
    }

    void clearListener() { m_cbs.clear(); }

  private:
    T m_value;
    std::map<uint64_t, changedCallBack> m_cbs;
};

class Config {
  public:
    typedef std::unordered_map<std::string, ConfigVarBase::ptr> ConfigVarMap;

    /**
     * @brief
     *
     * @param name
     * @param defaultValue
     * @param description
     * @return ConfigVar<T>::ptr
     */
    template <class T>
    static typename ConfigVar<T>::ptr
    Lookup(const std::string &name, const T &defaultValue,
           const std::string &description = "") {
        auto it = GetData().find(name);
        if (it != GetData().end()) {
            auto temp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
            if (temp) {
                SOLAR_LOG_INFO(SOLAR_LOG_ROOT())
                    << "Lookup name = " << name << " exist";
                return temp;
            } else {
                SOLAR_LOG_ERROR(SOLAR_LOG_ROOT())
                    << "Lookup name = " << name
                    << " exist, but type not :" << typeid(T).name()
                    << " real_type = " << it->second->getTypeName();
                return nullptr;
            }
        }
        if (name.find_first_not_of("abcdefghikjlmnopqrstuvwxyz._0123456789") !=
            std::string::npos) {
            SOLAR_LOG_ERROR(SOLAR_LOG_ROOT()) << "Lookup name invalid " << name;
            throw std::invalid_argument(name);
        }
        typename ConfigVar<T>::ptr v =
            std::make_shared<ConfigVar<T>>(name, defaultValue, description);
        GetData()[name] = v;
        return v;
    }
    template <class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string &name) {
        auto it = GetData().find(name);
        if (it == GetData().end()) {
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }

    static void LoadFromYaml(const YAML::Node &root);

    static ConfigVarBase::ptr LookupBase(const std::string &name);

   private:
    /**
     * @brief Get the Data object,
     * 不直接定义静态成员变量，而是定义一个静态成员函数返回函数中定义的静态变量。
     * 这样能够方便后面扩展，比如在函数中添加锁做成线程安全的版本。
     *
     * @return ConfigVarMap&
     */
    // 全局函数/静态函数大驼峰，成员函数小驼峰
    static ConfigVarMap &GetData() {
        static ConfigVarMap s_datas;
        return s_datas;
    }
};

} // namespace solar

#endif /* __SOLAR_UTIL_CONFIG_H__ */
