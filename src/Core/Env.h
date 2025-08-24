/**
 * @file Env.h
 * @brief 
 * @author guovsea
 * @email guovsea@gmail.com
 */
#ifndef __SOLAR_ENV_H__
#define __SOLAR_ENV_H__

#include <map>
#include <vector>
#include "Mutex.h"
#include "Util/Singleton.h"

namespace solar {
class Env {
public:
    typedef RWMutex RWMutexType;
    bool init(int argc, char* argv[]);

    void add(const std::string& key, const std::string& val);
    bool has(const std::string& key);
    void del(const std::string& key);
    std::string get(const std::string& key, const std::string& default_val = "");

    void addHelp(const std::string& key, const std::string& desc);
    void removeHelp(const std::string& key);
    void printHelp();
private:
    RWMutexType m_mutex;
    std::map<std::string, std::string> m_args;
    std::vector<std::pair<std::string, std::string>> m_helps;
    std::string m_program;
};

typedef Singleton<Env> EnvMgr;
};


#endif //__SOLAR_ENV_H__