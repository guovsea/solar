#include "Core/Hook.h"
#include "Core/Fiber.h"
#include "Core/IOManager.h"
#include <dlfcn.h>

namespace solar {

static thread_local bool t_hook_enable{false};

#define HOOK_FUN(XX)                                                           \
  XX(sleep)                                                                    \
  XX(usleep)

void hook_init() {
  static bool is_inited = false;
  if (is_inited) {
    return;
  }
  // sleep_f = (sleep_fun)dlsym(RTLD_NEXT, sleep);
#define XX(name) name##_f = (name##_fun)dlsym(RTLD_NEXT, #name);
  HOOK_FUN(XX);
#undef XX
}

struct _HookIniter {
  _HookIniter() { hook_init(); }
};

static _HookIniter s_hook_initer;

bool is_hook_enable() { return t_hook_enable; }

void set_hook_enable(bool flag) { t_hook_enable = flag; }

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
  //   iom->addTimer(seconds * 1000,
  // std::bind(&solar::IOManager::schedule, iom, fiber)); // bind 不支持模板函数
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

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
}
