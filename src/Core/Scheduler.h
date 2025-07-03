#ifndef __SOLAR_CORE_SCHEDULER_H__
#define __SOLAR_CORE_SCHEDULER_H__

#include "Core/Fiber.h"
#include "Core/Mutex.h"
#include "Core/Thread.h"

#include <functional>
#include <list>
#include <memory>
#include <vector>

namespace solar {
class Scheduler {
public:
  typedef std::shared_ptr<Scheduler> ptr;
  typedef Mutex MutexType;

  /**
   * @brief
   *
   * @param threads
   * @param use_caller 适当 caller 线程，把构造 Scheduler 的线程添加到线程池中
   */
  Scheduler(size_t threads = 1, bool use_caller = true,
            const std::string &name = "");
  virtual ~Scheduler();

  const std::string &getName() const { return m_name; }

  /**
   * @brief 获取当前线程池
   *
   * @return Scheduler*
   */
  static Scheduler *GetThis();
  /**
   * @brief 获取主协程
   *
   * @return Fiber*
   */
  static Fiber *GetMainFiber();

  void start();

  void stop();

  template <typename FiberOrCb> void schedule(FiberOrCb fc, int thread = -1) {
    bool need_tickle = false;
    {
      MutexType::ScopedLock lock(m_mutex);
      need_tickle = scheduleNoLock(fc, thread);
    }
    if (need_tickle) {
      tickle();
    }
  }

  /**
   * @brief 一次性添加一批任务
   * 1. 只加一次锁
   * 2. 保证该批任务能够按顺序执行
   * 3. 会将传入序列中的任务 move 进 scheduler
   * @tparam InputIterator
   * @param begin
   * @param end
   */
  template <typename InputIterator>
  void schedule(InputIterator begin, InputIterator end) {
    bool need_tickle = false;
    {
      MutexType::ScopedLock lock(m_mutex);
      while (begin != end) {
        need_tickle = scheduleNoLock(std::move(*begin)) || need_tickle;
        ++begin;
      }
      if (need_tickle) {
        tickle();
      }
    }
  }

protected:
  virtual void tickle();
  /**
   * @brief 执行协程调度的方法
   *
   */
  void run();

  virtual bool stopping();

  virtual void idle();

  /// @brief 线程 ID
  std::vector<int> m_threadIds;
  /// @brief 线程总数
  size_t m_threadCount{0};
  /// @brief 激活线程数量
  std::atomic<size_t> m_activeThreadCount{0};
  /// @brief 空闲线程数量
  std::atomic<size_t> m_idleThreadCount{0};
  /// @brief 是否已经停止
  bool m_stopping{true};
  /// @brief 自动停止
  bool m_autoStop{false};
  /// @brief 主线程 id, -1 表示不使用创建 Scheduler 的线程作为主线程
  int m_rootThread{0};

private:
  template <typename FiberOrCb> bool scheduleNoLock(FiberOrCb fc, int thread) {
    bool need_tickle = m_fibers.empty(); //< 任务队列为空，
    FiberAndThread ft(fc, thread);
    if (ft.fiber || ft.cb) {
      m_fibers.push_back(ft);
    }
    return need_tickle;
  }

private:
  struct FiberAndThread {
    Fiber::ptr fiber;
    std::function<void()> cb;
    int thread;
    FiberAndThread(Fiber::ptr f, int thr) : fiber{f}, thread{thr} {}
    /**
     * @brief 使用 Fiber 构造
     * 会使用 swap 将调用时智能指针置为空。防止调用时的 Fiber::ptr
     * 指针始终存在，Fiber 对象无法被析构. move 也能实现相同的效果，应该用
     * move
     *
     * @param f 指向 Fiber::ptr 的指针
     * @param thr 指定的线程 id
     */
    FiberAndThread(Fiber::ptr *f, int thr) : thread{thr} { fiber.swap(*f); }

    FiberAndThread(Fiber::ptr &&f, int thr)
        : thread{thr}, fiber{std::move(f)} {}

    FiberAndThread(std::function<void()> cb, int thr) : cb(cb), thread(thr) {}
    /**
     * @brief 使用 std::function 构造
     * 会使用 swap 将调用时的 std::function 置空, std::function
     * 也可能会持有一些资源。 move 也能实现相同的效果，应该用 move
     * @param cb
     * @param thr 指定的线程 id
     */
    FiberAndThread(std::function<void()> *cb, int thr) : thread(thr) {
      cb->swap(*cb);
    }
    // FiberAndThread(std::function<void()> &&cb, int thr)
    //     : cb{std::move(cb)}, thread(thr) {}

    FiberAndThread() = default;
    /**
     * @brief 重置数据
     */
    void reset() {
      fiber = nullptr;
      cb = nullptr;
      thread = -1;
    }
  };

  void setThis();

private:
  MutexType m_mutex;
  std::vector<Thread::ptr> m_threads;
  std::list<FiberAndThread> m_fibers; //< 任务队列
  std::string m_name;
  Fiber::ptr m_rootFiber;
};
} // namespace solar

#endif