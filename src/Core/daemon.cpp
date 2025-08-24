#include <unistd.h>
#include "daemon.h"

#include <cstring>
#include <sys/wait.h>

#include "Util/Util.h"
#include "Log/Log.h"
#include "Util/Config.h"

namespace solar {

static Logger::ptr g_logger = SOLAR_LOG_NAME("system");
static ConfigVar<uint32_t>::ptr g_daemon_restart_interval
    = Config::Lookup("daemon.restart_interval", (uint32_t)5, "daemon restart time");

static int real_start(int argc, char* argv[], std::function<int(int argc, char* argv[])> main_cb) {
    return main_cb(argc, argv);
}
static int real_daemon(int argc, char* argv[], std::function<int(int argc, char* argv[])> main_cb) {
    ProcessInfoMgr::Instance()->parent_id = getpid();
    ProcessInfoMgr::Instance()->parent_start_time = GetCurrentMS();
    while (true) {
        pid_t pid = fork();
        if (pid == 0) {
            // 子进程返回
            ProcessInfoMgr::Instance()->main_id = getpid();
            ProcessInfoMgr::Instance()->main_start_time = GetCurrentMS();
            SOLAR_LOG_INFO(g_logger) << "process start pid=" << getpid();
            return main_cb(argc, argv);
        } else if (pid < 0) {
            SOLAR_LOG_ERROR(g_logger) << "fork fail return=" << pid << " errno=" << errno
                << " errstr=" << strerror(errno);
            return -1;
        } else {
            // 父进程返回
            int status{ 0 };
            waitpid(pid, &status, 0);
            if (status) {
                SOLAR_LOG_ERROR(g_logger) << "child crash pid=" << pid
                    << " status=" << status;
            } else {
                // 正常退出
                SOLAR_LOG_INFO(g_logger) << "child finished pid=" << pid;
                return status;
            }
            ++ProcessInfoMgr::Instance()->restart_count;
            sleep(g_daemon_restart_interval->getValue()); // 等待子进程的资源释放
        }
    }
}

int start_daemon(int argc, char *argv[], std::function<int(int argc, char *argv[])> main_cb, bool is_daemon) {
    if (!is_daemon) {
        return real_start(argc, argv, main_cb);
    }
    return real_daemon(argc, argv, main_cb);
}
}
