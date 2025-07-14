#include "Core/Scheduler.h"
#include "Core/Hook.h"
#include "Core/Thread.h"
#include "Log/Log.h"
#include "Util/macro.h"

using namespace solar;

namespace {
solar::Logger::ptr g_logger = SOLAR_LOG_NAME("system");

thread_local Scheduler *t_scheduler{nullptr};
/// @brief 主协程，执行 run 方法的协程
thread_local Fiber *t_fiber{nullptr};

} // namespace
namespace solar {
Scheduler::Scheduler(size_t threads, bool use_caller, const std::string &name)
    : m_name{name} {
  SOLAR_ASSERT(threads > 0);
  if (use_caller) {
    Fiber::GetThis();
    --threads;

    SOLAR_ASSERT(GetThis() == nullptr);
    t_scheduler = this;

    // 在新协程上执行 run 函数
    m_rootFiber = std::make_shared<Fiber>([this]() { this->run(); }, 0, true);
    Thread::SetName(m_name);

    // 主协程是执行 run 方法的协程
    t_fiber = m_rootFiber.get();
    m_rootThread = solar::GetThreadId();
    m_threadIds.push_back(m_rootThread);
  } else {
    m_rootThread = -1;
  }
  m_threadCount = threads;
  // m_rootFiber 被创建了，但是 m_rootFiber
  // 协程还没有真正被执行，当前的执行流在当前线程的主协程（区别于调度器中的主协程：t_fiber)
}

Scheduler::~Scheduler() {
  SOLAR_ASSERT(m_stopping);
  if (GetThis() == this) {
    t_scheduler = nullptr;
  }
}

Fiber *Scheduler::GetMainFiber() { return t_fiber; }

void Scheduler::start() {
  MutexType::ScopedLock lock(m_mutex);
  if (!m_stopping) {
    return;
  }
  m_stopping = false;
  SOLAR_ASSERT(m_threads.empty());
  m_threads.resize(m_threadCount);
  for (size_t i = 0; i < m_threadCount; ++i) {
    m_threads[i] = std::make_shared<Thread>([this]() { this->run(); },
                                            m_name + "_" + std::to_string(i));
    m_threadIds.push_back(m_threads[i]->getId());
  }
  lock.unlock();
}

void Scheduler::stop() {
  m_autoStop = true;
  if (m_rootFiber && m_threadCount == 0 &&
      (m_rootFiber->getState() == Fiber::TERM ||
       m_rootFiber->getState() == Fiber::INIT)) {
    // 只有一个主线程在运行
    SOLAR_LOG_INFO(g_logger) << this << " stopped";
    m_stopping = true;
    if (stopping()) {
      return;
    }
  }
  // 有多个子线程正在运行
  if (m_rootThread != -1) {
    // use_caller 时必须在创建 Scheduler 的线程中 stop
    SOLAR_ASSERT(GetThis() == this);
  } else {
    SOLAR_ASSERT(GetThis() != this);
  }
  m_stopping = true;
  for (size_t i = 0; i < m_threadCount; ++i) {
    tickle();
  }
  if (m_rootFiber) {
    tickle();
    if (!stopping()) {
      m_rootFiber->call();
    }
  }
  std::vector<Thread::ptr> thrs;
  {
    MutexType::ScopedLock lock(m_mutex);
    thrs.swap(m_threads);
  }
  for (auto &i : thrs) {
    i->join();
  }
}

void Scheduler::tickle() { SOLAR_LOG_INFO(g_logger) << "tickle"; }

void Scheduler::run() {
  SOLAR_LOG_DEBUG(g_logger) << "run";
  set_hook_enable(true);
  setThis();
  if (solar::GetThreadId() != m_rootThread) {
    t_fiber = Fiber::GetThis().get();
  }
  Fiber::ptr idle_fiber = std::make_shared<Fiber>([this]() { this->idle(); });
  Fiber::ptr cb_fiber;

  FiberAndThread ft{};
  while (true) {
    ft.reset();
    bool tickle_me = false;
    bool is_active = false;
    {
      // 从任务队列中取出任务
      MutexType::ScopedLock lock(m_mutex);
      auto it = m_fibers.begin();
      while (it != m_fibers.end()) {
        if (it->thread != -1 && it->thread != solar::GetFiberId()) {
          ++it;
          tickle_me = true;
          continue;
        }
        SOLAR_ASSERT(it->fiber || it->cb);
        if (it->fiber && it->fiber->getState() == Fiber::EXEC) {
          ++it;
          continue;
        }
        ft = *it;
        m_fibers.erase(it);
        ++m_activeThreadCount;
        is_active = true;
        break;
      }
    }
    if (tickle_me) {
      tickle();
    }

    if (ft.fiber && (ft.fiber->getState() != Fiber::TERM &&
                     ft.fiber->getState() != Fiber::EXCEPT)) {
      // Fiber
      ft.fiber->swapIn();
      --m_activeThreadCount;
      if (ft.fiber->getState() == Fiber::READY) {
        schedule(ft.fiber);
      } else if (ft.fiber->getState() != Fiber::TERM &&
                 ft.fiber->getState() != Fiber::EXCEPT) {
        ft.fiber->m_state = Fiber::HOLD;
      }
      ft.fiber.reset();
    } else if (ft.cb) {
      // Call back
      if (cb_fiber) {
        cb_fiber->reset(ft.cb);
      } else {
        cb_fiber.reset(new Fiber(ft.cb));
      }
      cb_fiber->swapIn();
      --m_activeThreadCount;
      if (cb_fiber->getState() == Fiber::READY) {
        schedule(cb_fiber);
        cb_fiber.reset();
      } else if (cb_fiber->getState() == Fiber::EXCEPT ||
                 cb_fiber->getState() == Fiber::TERM) {
        cb_fiber->reset(nullptr);
      } else { // cb_fiber->getState() == Fiber::TERM)
        cb_fiber->m_state = Fiber::HOLD;
        cb_fiber.reset();
      }
    } else { // 没有任务
      if (is_active) {
        -m_activeThreadCount;
      }
      if (idle_fiber->getState() == Fiber::TERM) {
        SOLAR_LOG_INFO(g_logger) << "idle fiber term";
        break;
      }
      ++m_idleThreadCount;
      idle_fiber->swapIn();
      --m_idleThreadCount;
      // idle_fiber 并没有执行完
      if (idle_fiber->getState() != Fiber::TERM &&
          idle_fiber->getState() != Fiber::EXCEPT) {
        idle_fiber->m_state = Fiber::HOLD;
      }
    }
  }
}

bool Scheduler::stopping() {
  MutexType::ScopedLock lock(m_mutex);
  return m_autoStop && m_stopping && m_fibers.empty() &&
         m_activeThreadCount == 0;
}

void Scheduler::idle() {
  SOLAR_LOG_INFO(g_logger) << "idle";
  while (!stopping()) {
    Fiber::GetThis()->YeildToRead();
  }
}

void Scheduler::setThis() { t_scheduler = this; }

Scheduler *Scheduler::GetThis() { return t_scheduler; }

} // namespace solar