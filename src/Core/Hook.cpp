#include "Core/Hook.h"
#include <Log/Log.h>
#include <dlfcn.h>
#include "Core/Fiber.h"
#include "Core/IOManager.h"
#include "Core/fd_manager.h"
#include "Util/Config.h"
#include "stdarg.h"

namespace solar {
static Logger::ptr g_logger = SOLAR_LOG_NAME("system");
static solar::ConfigVar<int>::ptr g_tcp_connect_timeout =
        solar::Config::Lookup("tcp.connect.timeout", 5000, "tcp connect timeout");
static thread_local bool t_hook_enable{false};

#define HOOK_FUN(XX)                                                                                                   \
    XX(sleep)                                                                                                          \
    XX(usleep)                                                                                                         \
    XX(nanosleep)                                                                                                      \
    XX(socket)                                                                                                         \
    XX(connect)                                                                                                        \
    XX(accept)                                                                                                         \
    XX(read)                                                                                                           \
    XX(readv)                                                                                                          \
    XX(recv)                                                                                                           \
    XX(recvfrom)                                                                                                       \
    XX(recvmsg)                                                                                                        \
    XX(write)                                                                                                          \
    XX(writev)                                                                                                         \
    XX(send)                                                                                                           \
    XX(sendmsg)                                                                                                        \
    XX(sendto)                                                                                                         \
    XX(close)                                                                                                          \
    XX(fcntl)                                                                                                          \
    XX(ioctl)                                                                                                          \
    XX(getsockopt)                                                                                                     \
    XX(setsockopt)

void hook_init() {
    static bool is_inited = false;
    if (is_inited) {
        return;
    }
    //     sleep_f = (sleep_fun)dlsym(RTLD_NEXT, "sleep");
#define XX(name) name##_f = (name##_fun) dlsym(RTLD_NEXT, #name);
    HOOK_FUN(XX);
#undef XX
}

static uint64_t s_connect_timeout = -1;
struct _HookIniter {
    _HookIniter() {
        hook_init();
        s_connect_timeout = g_tcp_connect_timeout->getValue();
        g_tcp_connect_timeout->addListener(0x55585, [](const int &old_value, const int &new_value) {
            SOLAR_LOG_INFO(g_logger) << "tcp connect timeout changed: from " << old_value << " to " << new_value;
            s_connect_timeout = new_value;
        });
    }
};

static _HookIniter s_hook_initer;

bool is_hook_enable() { return t_hook_enable; }

void set_hook_enable(bool flag) { t_hook_enable = flag; }

struct timer_info {
    int cancelled{0};
};

template<typename OriginFun, typename... Args>
static ssize_t do_io(int fd, OriginFun fun, const char *hook_fun_name, uint32_t event, int timeout_so,
                     Args &&... args) {
    if (!solar::t_hook_enable) {
        return fun(fd, std::forward<Args>(args)...);
    }
    FdCtx::ptr ctx = FdMgr::Instance()->get(fd);
    if (!ctx) {
        return fun(fd, std::forward<Args>(args)...);
    }
    if (ctx->isClosed()) {
        errno = EBADF;
        return -1;
    }

    if (!ctx->isSocket() || ctx->getUserNonblock()) {
        return fun(fd, std::forward<Args>(args)...);
    }

    uint64_t to = ctx->getTimeout(timeout_so);
    std::shared_ptr<timer_info> tinfo(new timer_info);

retry:
    ssize_t n = fun(fd, std::forward<Args>(args)...);
    while (n == -1 && errno == EINTR) {
        n = fun(fd, std::forward<Args>(args)...);
    }
    if (n == -1 && errno == EAGAIN) {
        SOLAR_LOG_DEBUG(g_logger) << "do_io<" << hook_fun_name << ">";
        IOManager *iom = IOManager::GetThis();
        Timer::ptr timer;
        std::weak_ptr<timer_info> winfo(tinfo);

        if (to != ~0ull) {
            timer = iom->addConditionTimer(
                    to,
                    [winfo, fd, iom, event]() {
                        auto t = winfo.lock();
                        if (!t || t->cancelled) {
                            return;
                        }
                        t->cancelled = ETIMEDOUT;
                        iom->cancelEvent(fd, static_cast<IOManager::Event>(event));
                    },
                    winfo);
        }
        int rt = iom->addEvent(fd, static_cast<IOManager::Event>(event));
        if (rt) {
            SOLAR_LOG_ERROR(g_logger) << hook_fun_name << " addEvent(" << fd << ", " << event << ")";
            if (timer) {
                timer->cancel();
            }
            return -1;
        } else {
            SOLAR_LOG_DEBUG(g_logger) << "do_io<" << hook_fun_name << "> yield to hold";
            Fiber::YeildToHold();
            // 有数据到来，被 swapIn, 或者超时了被 cancel
            SOLAR_LOG_DEBUG(g_logger) << "do_io<" << hook_fun_name << "> swap in";
            if (timer) {
                timer->cancel();
            }
            if (tinfo->cancelled) {
                errno = tinfo->cancelled;
                return -1;
            }
            goto retry;
        }
    }
    return n;
}

int connect_with_timeout(int fd, const struct sockaddr *addr, socklen_t addrlen, uint64_t timeout_ms) {
    if (!solar::t_hook_enable) {
        return connect_f(fd, addr, addrlen);
    }
    solar::FdCtx::ptr ctx = solar::FdMgr::Instance()->get(fd);
    if (!ctx || ctx->isClosed()) {
        errno = EBADF;
        return -1;
    }
    if (!ctx->isSocket()) {
        return connect_f(fd, addr, addrlen);
    }
    int n = connect_f(fd, addr, addrlen);
    if (n == 0) {
        return 0;
    } else if (n != -1 || errno != EINPROGRESS) {
        return n;
    }
    solar::IOManager *iom = solar::IOManager::GetThis();
    solar::Timer::ptr timer;
    std::shared_ptr<timer_info> tinfo(new timer_info);
    std::weak_ptr<timer_info> winfo(tinfo);

    if (timeout_ms != ~0ull) {
        timer = iom->addConditionTimer(
                timeout_ms,
                [winfo, fd, iom]() {
                    auto t = winfo.lock();
                    if (!t || t->cancelled) {
                        return;
                    }
                    t->cancelled = ETIMEDOUT;
                    iom->cancelEvent(fd, solar::IOManager::WRITE);
                },
                winfo);
    }
    int rt = iom->addEvent(fd, solar::IOManager::WRITE);
    if (rt) {
        SOLAR_LOG_ERROR(g_logger) << "connect"
                                  << " addEvent(" << fd << ", WRITE) error";
        if (timer) {
            timer->cancel();
        }
        return -1;
    } else {
        Fiber::YeildToHold();
        // 有数据到来，被 swapIn, 或者超时了被 cancel
        if (timer) {
            timer->cancel();
        }
        if (tinfo->cancelled) {
            errno = tinfo->cancelled;
            return -1;
        }
    }
    int error = 0;
    socklen_t len = sizeof(int);
    if (-1 == getsockopt(fd, SOL_SOCKET, SO_ERROR, &errno, &len)) {
        return -1;
    }
    if (!error) {
        return 0;
    } else {
        errno = error;
        return -1;
    }
}

} // namespace solar

extern "C" {

#define XX(name) name##_fun name##_f = nullptr;
HOOK_FUN(XX);
#undef XX

unsigned int sleep(unsigned int seconds) {
    if (!solar::t_hook_enable) {
        return sleep_f(seconds);
    }
    solar::Fiber::ptr fiber = solar::Fiber::GetThis();
    solar::IOManager *iom = solar::IOManager::GetThis();
    iom->addTimer(seconds * 1000, [iom, fiber]() { iom->schedule(fiber); });
    fiber->YeildToHold();
    return 0;
}

int usleep(useconds_t usec) {
    if (!solar::t_hook_enable) {
        return sleep_f(usec);
    }
    solar::Fiber::ptr fiber = solar::Fiber::GetThis();
    solar::IOManager *iom = solar::IOManager::GetThis();
    iom->addTimer(usec / 1000, [iom, fiber]() { iom->schedule(fiber); });
    fiber->YeildToHold();
    return 0;
}

int nanosleep(const struct timespec *req, struct timespec *rem) {
    if (!solar::t_hook_enable) {
        return nanosleep(req, rem);
    }
    int timeout_ms = req->tv_sec * 1000 + req->tv_nsec / 1000 / 1000;
    solar::Fiber::ptr fiber = solar::Fiber::GetThis();
    solar::IOManager *iom = solar::IOManager::GetThis();
    iom->addTimer(timeout_ms, [iom, fiber]() { iom->schedule(fiber); });
    fiber->YeildToHold();
    return 0;
}

// socker
int socket(int domain, int type, int protocol) {
    if (!solar::t_hook_enable) {
        return socket_f(domain, type, protocol);
    }
    int fd = socket_f(domain, type, protocol);
    if (fd == -1) {
        return fd;
    }
    solar::FdMgr::Instance()->get(fd, true);
    return fd;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    return solar::connect_with_timeout(sockfd, addr, addrlen, solar::s_connect_timeout);
}

int accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
    int fd = solar::do_io(s, accept_f, "accept", solar::IOManager::READ, SO_RCVTIMEO, addr, addrlen);
    if (fd > 0) {
        solar::FdMgr::Instance()->get(fd, true);
    }
    return fd;
}

ssize_t read(int fd, void *buf, size_t count) {
    return solar::do_io(fd, read_f, "read", solar::IOManager::READ, SO_RCVTIMEO, buf, count);
}
size_t readv(int fd, const struct iovec *iov, int iovcnt) {
    return solar::do_io(fd, readv_f, "readv", solar::IOManager::READ, SO_RCVTIMEO, iov, iovcnt);
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    return solar::do_io(sockfd, recv_f, "recv", solar::IOManager::READ, SO_RCVTIMEO, buf, len, flags);
}

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
    return solar::do_io(sockfd, recvfrom_f, "recvfrom", solar::IOManager::READ, SO_RCVTIMEO, buf, len, flags, src_addr,
                        addrlen);
}

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
    return solar::do_io(sockfd, recvmsg_f, "recvmsg", solar::IOManager::READ, SO_RCVTIMEO, msg, flags);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return solar::do_io(fd, write_f, "write", solar::IOManager::WRITE, SO_SNDTIMEO, buf, count);
}

size_t writev(int fd, const struct iovec *iov, int iovcnt) {
    return solar::do_io(fd, writev_f, "writev", solar::IOManager::WRITE, SO_SNDTIMEO, iov, iovcnt);
}

ssize_t send(int s, const void *msg, size_t len, int flags) {
    return do_io(s, send_f, "send", solar::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags);
}

ssize_t sendto(int s, const void *msg, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) {
    return do_io(s, sendto_f, "sendto", solar::IOManager::WRITE, SO_SNDTIMEO, msg, len, flags, to, tolen);
}

ssize_t sendmsg(int s, const struct msghdr *msg, int flags) {
    return do_io(s, sendmsg_f, "sendmsg", solar::IOManager::WRITE, SO_SNDTIMEO, msg, flags);
}

int close(int fd) {
    if (!solar::t_hook_enable) {
        return close_f(fd);
    }

    solar::FdCtx::ptr ctx = solar::FdMgr::Instance()->get(fd);
    if (ctx) {
        auto iom = solar::IOManager::GetThis();
        if (iom) {
            iom->cancelAll(fd);
        }
        solar::FdMgr::Instance()->del(fd);
    }
    return close_f(fd);
}

int fcntl(int fd, int cmd, ... /* arg */) {
    va_list va;
    va_start(va, cmd);
    switch (cmd) {
        case F_SETFL: {
            int arg = va_arg(va, int);
            va_end(va);
            solar::FdCtx::ptr ctx = solar::FdMgr::Instance()->get(fd);
            if (!ctx || ctx->isClosed() || !ctx->isSocket()) {
                return fcntl_f(fd, cmd, arg);
            }
            ctx->setUserNonblock(arg & O_NONBLOCK);
            if (ctx->getSysNonblock()) {
                arg |= O_NONBLOCK;
            } else {
                arg &= ~O_NONBLOCK;
            }
            return fcntl_f(fd, cmd, arg);
        } break;
        case F_GETFL: {
            va_end(va);
            int arg = fcntl(fd, cmd);
            solar::FdCtx::ptr ctx = solar::FdMgr::Instance()->get(fd);
            if (!ctx || ctx->isClosed() || !ctx->isSocket()) {
                return arg;
            }
            if (ctx->getUserNonblock()) {
                return arg | O_NONBLOCK;
            } else {
                return arg & ~O_NONBLOCK;
            }
        } break;
        case F_DUPFD:
        case F_DUPFD_CLOEXEC:
        case F_SETFD:
        case F_SETOWN:
        case F_SETSIG:
        case F_SETLEASE:
        case F_NOTIFY:
        case F_SETPIPE_SZ: {
            int arg = va_arg(va, int);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        } break;

        case F_GETFD:
        case F_GETOWN:
        case F_GETSIG:
        case F_GETLEASE:
        case F_GETPIPE_SZ: {
            va_end(va);
            return fcntl_f(fd, cmd);
        } break;

        case F_SETLK:
        case F_SETLKW:
        case F_GETLK: {
            struct flock *arg = va_arg(va, struct flock *);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        } break;

        case F_GETOWN_EX:
        case F_SETOWN_EX: {
            struct f_owner_exlock *arg = va_arg(va, struct f_owner_exlock *);
            va_end(va);
            return fcntl_f(fd, cmd, arg);
        } break;

        default:
            va_end(va);
            return fcntl_f(fd, cmd);
    }
}

int ioctl(int d, unsigned long int request, ...) {
    va_list va;
    va_start(va, request);
    void *arg = va_arg(va, void *);
    va_end(va);
    if (FIONBIO == request) {
        bool user_nonblock = !!*(int *) arg;
        solar::FdCtx::ptr ctx = solar::FdMgr::Instance()->get(d);
        if (!ctx || ctx->isClosed() || !ctx->isSocket()) {
            return ioctl_f(d, request, arg);
        }
        ctx->setUserNonblock(user_nonblock);
    }
    return ioctl_f(d, request, arg);
}

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen) {
    return getsockopt_f(sockfd, level, optname, optval, optlen);
}
int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen) {
    if (!solar::t_hook_enable) {
        return setsockopt_f(sockfd, level, optname, optval, optlen);
    }
    if (level == SOL_SOCKET) {
        if (optname == SO_RCVTIMEO || optname == SO_SNDTIMEO) {
            solar::FdCtx::ptr ctx = solar::FdMgr::Instance()->get(sockfd);
            if (ctx) {
                const timeval *tv = (const timeval *) optval;
                ctx->setTimeout(optname, tv->tv_sec * 1000 + tv->tv_usec / 1000);
            }
        }
    }
    return setsockopt_f(sockfd, level, optname, optval, optlen);
}
}