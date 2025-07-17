/**
 * @file Hook.h
 * @brief Hooking 网络编程中常用的系统调用
 * @version 0.1
 * @date 2025-07-15
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef __SOLAR_CORE_HOOK_H__
#define __SOLAR_CORE_HOOK_H__

#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

namespace solar {
bool is_hook_enable();
void set_hook_enable(bool falg);

} // namespace solar

extern "C" {

// sleep
typedef unsigned int (*sleep_fun)(unsigned int seconds);
extern sleep_fun sleep_f;

typedef int (*usleep_fun)(useconds_t usec);
extern usleep_fun usleep_f;

typedef int (*nanosleep_fun)(const struct timespec *req, struct timespec *rem);
extern nanosleep_fun nanosleep_f;

// socket
typedef int (*socket_fun)(int domain, int type, int protocol);
extern socket_fun socket_f;
typedef int (*connect_fun)(int sockfd, const struct sockaddr *addr,
                           socklen_t addrlen);
extern connect_fun connect_f;
typedef int (*accept_fun)(int s, struct sockaddr *addr, socklen_t addrlen);
extern accept_fun accept_f;

typedef ssize_t (*read_fun)(int fd, void *buf, size_t count);
extern read_fun read_f;
typedef size_t (*readv_fun)(int fd, const struct iovec *iov, int iovcnt);
extern readv_fun readv_f;

typedef ssize_t (*recv_fun)(int sockfd, void *buf, size_t len, int flags);
extern recv_fun recv_f;

typedef ssize_t (*recvfrom_fun)(int sockfd, void *buf, size_t len, int flags,
                                struct sockaddr *src_addr, socklen_t *addrlen);
ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags);
}

#endif
