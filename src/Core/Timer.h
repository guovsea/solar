#ifndef __SOLAR_CORE_TIMER_H__
#define __SOLAR_CORE_TIMER_H__

#include "Core/Mutex.h"
#include <functional>
#include <memory>
#include <set>

namespace solar {
class TimerManager;
class Timer : public std::enable_shared_from_this<Timer> {
  friend class TimerManager;

public:
  typedef std::shared_ptr<Timer> ptr;

private:
  /**
   * @brief
   *
   * @param ms
   * @param cb
   * @param recurring  是否是循环定时器
   * @param manager
   */
  Timer(uint64_t ms, std::function<void()> cb, bool recurring,
        TimerManager *manager);

  Timer(uint64_t next);

  /**
   * @brief 取消定时器
   *
   * @return true 取消成功
   * @return false 被取消或被从管理器中拿出
   */
  bool cancel();

  /**
   * @brief 刷新定时器，在当前时间点再设定一个原时长的定时器
   *
   * @return true 重置成功
   * @return false 被取消或被从管理器中拿出
   */
  bool refresh();
  /**
   * @brief 重置定时器
   *
   * @param ms 重置后的定时事件
   * @param from_now 是否从当前时刻开始
   * @return true
   * @return false 被取消或被从管理器中拿出
   */
  bool reset(uint64_t ms, bool from_now);

private:
  struct Comparator {
    bool operator()(const Timer::ptr &lhs, const Timer::ptr &rhs) const;
  };

  uint64_t m_ms{0};                 //< 定时器的时间周期
  std::function<void()> m_cb;       //< 定时器的回调函数
  bool m_recurring{false};          //< 是否是循环定时器
  uint64_t m_next{0};               //< 下次触发的时间点，绝对时间
  TimerManager *m_manager{nullptr}; //< 定时器管理器
};

class TimerManager {
  friend class Timer;

public:
  typedef RWMutex RWMutexType;
  typedef std::shared_ptr<TimerManager> ptr;

  TimerManager();
  virtual ~TimerManager();
  /**
   * @brief 添加计时器
   *
   * @param ms
   * @param cb 超时回调函数
   * @param recurring 是否是循环定时器
   * @return Timer::ptr
   */
  Timer::ptr addTimer(uint64_t ms, std::function<void()> cb,
                      bool recurring = false);
  /**
   * @brief 添加条件计时器
   *
   * @param ms
   * @param cb
   * @param weak_cond 当 weak_cond->lock() 成功时才执行超时回调函数
   * @param recurring 是否是循环定时器
   * @return Timer::ptr
   */
  Timer::ptr addConditionTimer(uint64_t ms, std::function<void()> cb,
                               std::weak_ptr<void> weak_cond,
                               bool recurring = false);
  uint64_t getNextTimer();
  void listExpiredCb(std::vector<std::function<void()>> &cbs);

protected:
  virtual void onTimerInsertedAtFront() = 0;
  void addTimer(Timer::ptr timer, RWMutexType::WriteScopedLock &lock);

private:
  RWMutexType m_mutex;           //< 保护定时器的互斥锁
  std::set<Timer::ptr> m_timers; //< 定时器集合
};
} // namespace solar

#endif // __SOLAR_CORE_TIMER_H__