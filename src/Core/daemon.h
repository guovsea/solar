/**
 * @file Daemon.h
 * @brief 
 * @author guovsea
 * @email guovsea@gmail.com
 */
#ifndef __SOLAR_DAEMON_H__
#define __SOLAR_DAEMON_H__
#include <functional>
#include "Util/Singleton.h"

namespace solar {
struct ProcessInfo {
    pid_t parent_id = 0;
    pid_t main_id = 0;
    uint64_t parent_start_time = 0;
    uint64_t main_start_time = 0;
    uint64_t restart_count = 0;

    std::string toString() const;
};
typedef Singleton<ProcessInfo> ProcessInfoMgr;
int start_daemon(int argc, char* argv[], std::function<int(int argc, char* argv[])> main_cb, bool is_daemon);
}

#endif //__SOLAR_DAEMON_H__