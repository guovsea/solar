#include "Core/Timer.h"
#include "Timer.h"
#include "Util/Util.h"

namespace solar {
void OnTimer(std::weak_ptr<void> weak_cond, std::function<void()> cb) {
  std::shared_ptr<void> tmp = weak_cond.lock();
  if (tmp) {
    cb();
  }
}
} // namespace solar

namespace solar {
bool Timer::Comparator::operator()(const Timer::ptr &lhs,
                                   const Timer::ptr &rhs) const {
  if (!lhs || !rhs) {
    return false;
  }
  if (!lhs) {
    return true;
  }
  if (!rhs) {
    return false;
  }
  // 按照下次触发时间点升序排列
  if (lhs->m_next < rhs->m_next) {
    return true;
  } else if (lhs->m_next > rhs->m_next) {
    return false;
  }
  // 如果时间相同，按地址排序
  return lhs.get() < rhs.get();
}

Timer::Timer(uint64_t ms, std::function<void()> cb, bool recurring,
             TimerManager *manager)
    : m_ms{ms}, m_cb{cb}, m_recurring{recurring}, m_manager{manager} {
  m_next = GetCurrentMS() + ms;
}

Timer::Timer(uint64_t next) : m_next{next} {}

bool Timer::cancel() {
  TimerManager::RWMutexType::WriteScopedLock lock(m_manager->m_mutex);
  if (m_cb) {
    m_cb = nullptr;
    auto it = m_manager->m_timers.find(shared_from_this());
    m_manager->m_timers.erase(it);
    return true;
  }
  return false;
}

bool Timer::refresh() {
  TimerManager::RWMutexType::WriteScopedLock lock(m_manager->m_mutex);
  if (!m_cb) {
    return false;
  }
  auto it = m_manager->m_timers.find(shared_from_this());
  if (it == m_manager->m_timers.end()) {
    return false;
  }
  // 先移除，再删除。因为如果直接修改，那么 it 在 m_timers
  // 中的位置不会变，但实际上它应该放在 set 中的另一个位置，m_timers 就坏掉了
  m_manager->m_timers.erase(it);
  m_next = GetCurrentMS() + m_ms;
  // 重置不会插入到最前面
  m_manager->addTimer(shared_from_this(), lock);
  return true;
}

bool Timer::reset(uint64_t ms, bool from_now) {
  if (ms == m_ms && !from_now) {
    return true;
  }
  TimerManager::RWMutexType::WriteScopedLock lock(m_manager->m_mutex);
  if (!m_cb) {
    return false;
  }
  auto it = m_manager->m_timers.find(shared_from_this());
  if (it == m_manager->m_timers.end()) {
    return false;
  }
  m_manager->m_timers.erase(it);
  uint64_t start{0};
  if (from_now) {
    start = GetCurrentMS();
  } else {
    start = m_next - m_ms;
  }
  m_ms = ms;
  m_next = start + m_ms;
  // reset 可能会插入对最前面
  m_manager->addTimer(shared_from_this(), lock);
  return true;
}

TimerManager::TimerManager() { m_previousTime = GetCurrentMS(); }

Timer::ptr TimerManager::addTimer(uint64_t ms, std::function<void()> cb,
                                  bool recurring) {
  Timer::ptr timer = Timer::ptr{new Timer{ms, cb, recurring, this}};
  RWMutexType::WriteScopedLock lock(m_mutex);
  addTimer(timer, lock);
  return timer;
}

Timer::ptr TimerManager::addConditionTimer(uint64_t ms,
                                           std::function<void()> cb,
                                           std::weak_ptr<void> weak_cond,
                                           bool recurring) {
  return addTimer(ms, std::bind(&OnTimer, weak_cond, cb), recurring);
  return Timer::ptr();
}

uint64_t TimerManager::getNextTimer() {
  RWMutexType::ReadScopedLock lock(m_mutex);
  m_ticked = false;
  if (m_timers.empty()) {
    return ~0ull; // 没有定时器, 返回最大值
  }
  const Timer::ptr &next = *m_timers.begin();
  uint64_t now_ms = GetCurrentMS();
  if (now_ms >= next->m_next) {
    return 0;
  } else {
    return next->m_next - now_ms; // 返回下一个定时器的剩余时间
  }
}

void TimerManager::listExpiredCb(std::vector<std::function<void()>> &cbs) {
  uint64_t now_ms = GetCurrentMS();
  std::vector<Timer::ptr> expired;
  {
    RWMutexType::ReadScopedLock lock(m_mutex);
    if (m_timers.empty()) {
      return;
    }
  }
  RWMutex::WriteScopedLock lock(m_mutex);

  bool rollover = detectClockRollover(now_ms);
  if (!rollover && (*m_timers.begin())->m_next > now_ms) {
    return;
  }
  Timer::ptr now_timer{new Timer{now_ms}};
  // it 将指向下一个 > now_ms 的 timer，或者是 end()
  auto it = rollover ? m_timers.end() : m_timers.lower_bound(now_timer);
  while (it != m_timers.end() && (*it)->m_next == now_ms) {
    ++it;
  }

  expired.insert(expired.begin(), m_timers.begin(), it);
  m_timers.erase(m_timers.begin(), it);
  cbs.reserve(expired.size());

  for (auto &timer : expired) {
    cbs.push_back(timer->m_cb);
    if (timer->m_recurring) {
      timer->m_next = now_ms + timer->m_ms;
      m_timers.insert(timer);
    } else {
      timer->m_cb = nullptr;
    }
  }
}

void TimerManager::addTimer(Timer::ptr timer,
                            RWMutexType::WriteScopedLock &lock) {
  auto it = m_timers.insert(timer).first;
  // 如果是插入到最前面, 唤醒 epoll_wait 重新设置定时器
  bool at_front = (it == m_timers.begin());
  lock.unlock();

  bool need_tickle = at_front && !m_ticked;
  if (need_tickle) {
    onTimerInsertedAtFront();
  }
}

bool TimerManager::hasTimer() {
  RWMutexType::ReadScopedLock lock(m_mutex);
  return !m_timers.empty();
}

bool TimerManager::detectClockRollover(uint64_t now_ms) {
  bool rollover{false};
  if (now_ms < m_previousTime && now_ms < (m_previousTime - 60 * 60 * 1000)) {
    rollover = true;
  }
  m_previousTime = now_ms;
  return rollover;
}

} // namespace solar