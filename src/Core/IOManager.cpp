#include "Core/IOManager.h"
#include "IOManager.h"
#include "Log/Log.h"
#include "Util/macro.h"
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>

namespace {
solar::Logger::ptr g_logger = SOLAR_LOG_NAME("system");
}

namespace solar {

IOManager::FdContext::EventContext &
IOManager::FdContext::getContext(Event event) {
  switch (event) {
  case IOManager::READ:
    return read;
    break;
  case IOManager::WRITE:
    return write;
    break;
  default:
    SOLAR_ASSERT2(false, "getContext");
    break;
  }
}
void IOManager::FdContext::resetContext(EventContext &ctx) {
  ctx.scheduler = nullptr;
  ctx.fiber.reset();
  ctx.cb = nullptr;
}

void IOManager::FdContext::triggerEvent(Event event) {
  SOLAR_ASSERT(events & event);
  events = static_cast<Event>(events & ~event);
  EventContext &ctx = getContext(event);
  if (ctx.cb) {
    ctx.scheduler->schedule(ctx.cb);
    ctx.cb = nullptr;
  } else {
    ctx.scheduler->schedule(ctx.fiber);
    ctx.fiber = nullptr;
  }
  ctx.scheduler = nullptr;
  return;
}
IOManager::IOManager(size_t threads, bool use_caller, const std::string &name)
    : Scheduler{threads, use_caller, name} {
  m_epfd = epoll_create(5000);
  SOLAR_ASSERT(m_epfd > 0);
  int rt = pipe(m_tickleFds);
  SOLAR_ASSERT(rt);

  epoll_event event;
  memset(&event, 0, sizeof(epoll_event));
  event.events = EPOLLIN | EPOLLET;
  event.data.fd = m_tickleFds[0];
  rt = fcntl(m_tickleFds[0], F_SETFL, O_NONBLOCK);
  SOLAR_ASSERT(rt);

  rt = epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_tickleFds[0], &event);
  SOLAR_ASSERT(rt);

  resizeContexts(32);

  start();
}

IOManager::~IOManager() {
  stop();
  close(m_epfd);
  close(m_tickleFds[0]);
  close(m_tickleFds[1]);
  for (size_t i = 0; i < m_fdContexts.size(); ++i) {
    if (m_fdContexts[i]) {
      delete m_fdContexts[i];
    }
  }
}
int IOManager::addEvent(int fd, Event event, std::function<void()> cb) {
  FdContext *fd_ctx = nullptr;
  RWMutexType::ReadScopedLock lock(m_mutex);
  if (m_fdContexts.size() > fd) {
    fd_ctx = m_fdContexts[fd];
    lock.unlock();
  } else {
    lock.unlock();
    RWMutexType::WriteScopedLock lock2(m_mutex);
    resizeContexts(m_fdContexts.size() * 1.5);
    fd_ctx = m_fdContexts[fd];
  }

  FdContext::MutexType::ScopedLock lock2(fd_ctx->mutex);
  // 再次添加相同的事件，错误
  if (fd_ctx->events & event) {
    SOLAR_LOG_ERROR(g_logger)
        << "addEvent assert fd=" << fd << " event=" << event
        << " fd_ctx.event = " << fd_ctx->events;
    SOLAR_ASSERT(!(fd_ctx->events & event));
  }

  epoll_event epevent;
  epevent.events = EPOLLET | fd_ctx->events | event;
  epevent.data.ptr = fd_ctx;
  int op = fd_ctx->events ? EPOLL_CTL_ADD : EPOLL_CTL_ADD;
  int rt = epoll_ctl(m_epfd, op, fd, &epevent);

  if (rt) {
    SOLAR_LOG_ERROR(g_logger)
        << "epoll_ctl(" << m_epfd << ", " << op << "," << fd << ","
        << epevent.events << "):" << rt << " (" << errno << ") ("
        << strerror(errno) << ")";
    return -1;
  }

  ++m_pendingEventCount;
  fd_ctx->events = static_cast<Event>(fd_ctx->events | event);
  FdContext::EventContext &event_ctx = fd_ctx->getContext(event);
  // 新添加的 FdContext 应该都是无效的
  SOLAR_ASSERT(!event_ctx.scheduler && !event_ctx.fiber && !event_ctx.cb);

  event_ctx.scheduler = Scheduler::GetThis();
  if (cb) {
    event_ctx.cb.swap(cb);
  } else {
    event_ctx.fiber = Fiber::GetThis();
    SOLAR_ASSERT(event_ctx.fiber->getState() == Fiber::EXEC);
  }
  return 0;
}

bool IOManager::delEvent(int fd, Event event) {
  RWMutexType::ReadScopedLock lock(m_mutex);
  if (m_fdContexts.size() <= fd) {
    return false;
  }
  FdContext *fd_ctx = m_fdContexts[fd];
  lock.unlock();

  FdContext::MutexType::ScopedLock lock2(fd_ctx->mutex);
  if (!(fd_ctx->events & event)) {
    return false;
  }

  // 删除
  Event new_events = static_cast<Event>(fd_ctx->events & ~event);
  epoll_event epevent;
  epevent.events = EPOLLET | new_events;
  epevent.data.ptr = fd_ctx;
  int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
  int rt = epoll_ctl(m_epfd, op, fd, &epevent);

  if (rt) {
    SOLAR_LOG_ERROR(g_logger)
        << "epoll_ctl(" << m_epfd << ", " << op << "," << fd << ","
        << epevent.events << "):" << rt << " (" << errno << ") ("
        << strerror(errno) << ")";
    return false;
  }

  --m_pendingEventCount;
  fd_ctx->events = new_events;
  FdContext::EventContext &event_ctx = fd_ctx->getContext(event);
  fd_ctx->resetContext(event_ctx);
  return true;
}

bool IOManager::cancelEvent(int fd, Event event) {
  RWMutexType::ReadScopedLock lock(m_mutex);
  if (m_fdContexts.size() <= fd) {
    return false;
  }
  FdContext *fd_ctx = m_fdContexts[fd];
  lock.unlock();

  FdContext::MutexType::ScopedLock lock2(fd_ctx->mutex);
  if (!(fd_ctx->events & event)) {
    return false;
  }

  // 删除
  Event new_events = static_cast<Event>(fd_ctx->events & ~event);
  epoll_event epevent;
  epevent.events = EPOLLET | new_events;
  epevent.data.ptr = fd_ctx;
  int op = new_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
  int rt = epoll_ctl(m_epfd, op, fd, &epevent);

  if (rt) {
    SOLAR_LOG_ERROR(g_logger)
        << "epoll_ctl(" << m_epfd << ", " << op << "," << fd << ","
        << epevent.events << "):" << rt << " (" << errno << ") ("
        << strerror(errno) << ")";
    return false;
  }

  FdContext::EventContext &event_ctx = fd_ctx->getContext(event);
  fd_ctx->triggerEvent(event);
  --m_pendingEventCount;
  return true;
}

bool IOManager::cancelAll(int fd) {
  RWMutexType::ReadScopedLock lock(m_mutex);
  if (m_fdContexts.size() <= fd) {
    return false;
  }
  FdContext *fd_ctx = m_fdContexts[fd];
  lock.unlock();

  FdContext::MutexType::ScopedLock lock2(fd_ctx->mutex);
  if (!fd_ctx->events) {
    return false;
  }

  // 删除
  epoll_event epevent;
  epevent.events = 0;
  epevent.data.ptr = fd_ctx;
  int op = EPOLL_CTL_DEL;
  int rt = epoll_ctl(m_epfd, op, fd, &epevent);

  if (rt) {
    SOLAR_LOG_ERROR(g_logger)
        << "epoll_ctl(" << m_epfd << ", " << op << "," << fd << ","
        << epevent.events << "):" << rt << " (" << errno << ") ("
        << strerror(errno) << ")";
    return false;
  }

  if (fd_ctx->events & READ) {
    FdContext::EventContext &event_ctx = fd_ctx->getContext(READ);
    fd_ctx->triggerEvent(READ);
    --m_pendingEventCount;
  }
  if (fd_ctx->events & WRITE) {
    FdContext::EventContext &event_ctx = fd_ctx->getContext(WRITE);
    fd_ctx->triggerEvent(WRITE);
    --m_pendingEventCount;
  }
  // trigger 之后，事件应该都会被取消
  SOLAR_ASSERT(fd_ctx->events == 0);
  return true;
}

void IOManager::resizeContexts(size_t size) {
  m_fdContexts.resize(size);
  for (size_t i = 0; i < m_fdContexts.size(); ++i) {
    if (!m_fdContexts[i]) {
      m_fdContexts[i] = new FdContext;
      m_fdContexts[i]->fd = i;
    }
  }
}

IOManager *IOManager::GetThis() {
  return dynamic_cast<IOManager *>(Scheduler::GetThis());
}

void IOManager::tickle() {
  if (!hasIdleThreads()) {
    return;
  }
  int rt = write(m_tickleFds[1], "T", 1);
  SOLAR_ASSERT(rt == 1);
}

bool IOManager::stopping() {
  return Scheduler::stopping() && m_pendingEventCount == 0;
}

void IOManager::idle() {
  epoll_event *events = new epoll_event[64]{};
  std::shared_ptr<epoll_event> shared_events(events);
  while (true) {
    if (stopping()) {
      SOLAR_LOG_INFO(g_logger) << "name=" << getName() << " idle stopping exit";
      break;
    }
    int rt = 0;
    do {
      static const int MAX_TIMEOUT = 5000;
      rt = epoll_wait(m_epfd, events, 64, MAX_TIMEOUT);
      if (rt < 0 && errno == EINTR) {
        continue;
      } else {
        break;
      }
    } while (true);
    for (int i = 0; i < rt; ++i) {
      epoll_event &event = events[i];
      if (event.data.fd == m_tickleFds[0]) {
        uint8_t dummy;
        while (read(m_tickleFds[0], &dummy, 1) == 1)
          ; // 读干净，因为时 ET
        continue;
      }
      FdContext *fd_ctx = static_cast<FdContext *>(event.data.ptr);
      FdContext::MutexType::ScopedLock lock(fd_ctx->mutex);
      if (event.events & (EPOLLERR | EPOLLHUP)) {
        event.events |= EPOLLIN | EPOLLOUT;
      }
      int real_events = NONE;
      if (event.events & EPOLLIN) {
        real_events | READ;
      }
      if (event.events & EPOLLOUT) {
        real_events | WRITE;
      }
      // 如果没有事件
      if ((fd_ctx->events & real_events) == NONE) {
        continue;
      }

      // 剩余事件
      int left_events = (fd_ctx->events & ~real_events);
      int op = left_events ? EPOLL_CTL_MOD : EPOLL_CTL_DEL;
      event.events = EPOLLET | left_events;

      int rt2 = epoll_ctl(m_epfd, op, fd_ctx->fd, &event);
      if (rt2) {
        SOLAR_LOG_ERROR(g_logger)
            << "epoll_ctl(" << m_epfd << ", " << op << "," << fd_ctx->fd << ","
            << event.events << "):" << rt2 << " (" << errno << ") ("
            << strerror(errno) << ")";
        continue;
      }
      if (real_events & READ) {
        fd_ctx->triggerEvent(READ);
        --m_pendingEventCount;
      }
      if (real_events & WRITE) {
        fd_ctx->triggerEvent(WRITE);
        --m_pendingEventCount;
      }
    }
    // 处理完一轮事件让出循环
    Fiber::ptr cur = Fiber::GetThis();
    auto raw_ptr = cur.get();
    cur.reset();
    raw_ptr->swapOut();
  }
}

} // namespace solar