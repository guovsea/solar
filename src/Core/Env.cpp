#include <cstring>
#include <iostream>
#include <iomanip>

#include "Log/Log.h"
#include "Env.h"

namespace solar {
Logger::ptr g_logger = SOLAR_LOG_NAME("system");

bool Env::init(int argc, char *argv[]) {
    m_program = argv[0];
    // -config /path/to/config -file xxxx
    const char* now_key = nullptr;
    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if (strlen(argv[1]) > 1) {
                if (now_key) {
                    add(now_key, "");
                }
                now_key = argv[i] + 1;
            } else {
                SOLAR_LOG_ERROR(g_logger) << "invalid arg idx=" << i
                    << " val=" << argv[i];
                return false;
            }
        } else {
            if (now_key) {
                add(now_key, argv[i]);
                now_key = nullptr;
            } else {
                SOLAR_LOG_ERROR(g_logger) << "invalid arg idx=" << i
                    << " val=" << argv[i];
            }
        }
    }
    if (now_key) {
        add(now_key, "");
    }
    return true;
}

void Env::add(const std::string &key, const std::string &val) {
    RWMutexType::WriteLock lock(m_mutex);
    m_args[key] = val;
}

bool Env::has(const std::string &key) {
    RWMutexType::ReadLock lock(m_mutex);
    auto it = m_args.find(key);
    return it != m_args.end();
}

void Env::del(const std::string &key) {
    RWMutexType::WriteLock lock(m_mutex);
    auto it = m_args.find(key);
    m_args.erase(it);
}

std::string Env::get(const std::string &key, const std::string &default_val) {
    RWMutexType::ReadLock lock(m_mutex);
    auto it = m_args.find(key);
    return it != m_args.end() ? it->second : default_val;
}

void Env::addHelp(const std::string &key, const std::string &desc) {
    removeHelp(key);
    RWMutexType::ReadLock lock(m_mutex);
    m_helps.emplace_back(key, desc);
}

void Env::removeHelp(const std::string &key) {
    RWMutexType::WriteLock lock(m_mutex);
    m_helps.erase(std::remove_if(m_helps.begin(), m_helps.end(),
        [key](const std::pair<std::string, std::string>& i) {
                return i.first == key;
        }), m_helps.end());
}

void Env::printHelp() {
    RWMutexType::ReadLock lock(m_mutex);
    std::cout << "Usage: " << m_program << " [options]" << std::endl;
    for (auto& i : m_helps) {
        std::cout << std::setw(5) << "-" << i.first << " : " << i.second << std::endl;
    }
}
}

