#ifndef __SOLAR_CORE_FIBER_H__
#define __SOLAR_CORE_FIBER_H__
#include <functional>
#include <memory>
#include <ucontext.h>

#include "Core/Thread.h"

namespace solar {
/**
 * @brief 不能在栈上创建 Fiber 对象
 *
 */
class Fiber : public std::enable_shared_from_this<Fiber> {
public:
  typedef std::shared_ptr<Fiber> ptr;
  enum State { INIT, HOLD, EXEC, TERM, READY, EXCEPT };
  Fiber(std::function<void()> cb, size_t stackSize = 0);
  ~Fiber();
  /**
   * @brief 重置协程函数并重置状态，当协程状态为 INIT，TERM
   *
   * @param cb
   */
  void reset(std::function<void()> cb);
  // 让出执行权
  /**
   * @brief 切换到当前协程执行
   *
   */
  void swapIn();
  /**
   * @brief 当前协程道后台执行
   *
   */
  void swapOut();

  uint64_t getID() const { return m_id; }

public:
  /**
   * @brief 设置当前协程
   *
   * @param f
   */
  static void SetThis(Fiber *f);

  static Fiber::ptr GetThis();
  /**
   * @brief 当前协程切换到后台，并且设置为 Read 状态
   *
   */
  static void YeildToRead();
  /**
   * @brief 当前协程切换到后台，并且设置为 Hold 状态
   *
   */
  static void YeildToHold();
  static uint64_t TotalFibers();

  /**
   * @brief 主协程执行的函数
   *
   */
  static void MainFunc();

  static uint64_t GetFiberID();

  Fiber();

private:
  /**
   * @brief 主协程的构造函数
   *
   */
  uint64_t m_id{0};
  uint32_t m_stacksize{0};
  State m_state{INIT};
  ucontext_t m_ctx;
  void *m_stack = nullptr;
  std::function<void()> m_cb;
};

} // namespace solar

#endif