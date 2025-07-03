#include "Fiber.h"
#include "Core/Scheduler.h"
#include "Util/Config.h"
#include "Util/macro.h"
#include <atomic>
#include <exception>

using namespace solar;

namespace {
Logger::ptr g_logger = SOLAR_LOG_NAME("system");
// 统计所有线程的所有协程
std::atomic<uint64_t> s_fiber_id{0};
std::atomic<uint64_t> s_fiber_count{0};

// 当前线程中的前协程
thread_local Fiber *t_fiber{nullptr};
// 当前线程中的主协程
thread_local Fiber::ptr t_threadFiber{nullptr};

ConfigVar<uint32_t>::ptr g_fiber_stack_size = Config::Lookup<uint32_t>(
    "fiber.stack_size", 1024 * 1024, "fiber stack size");

} // namespace
namespace solar {

class MallocStackAllocator {
public:
  static void *Alloc(size_t size) { return malloc(size); }

  /**
   * @brief
   *
   * @param vp
   * @param size  预留给扩展
   */
  static void Dealloc(void *vp, size_t size) { return free(vp); }
};
using StackAllocator = MallocStackAllocator;

Fiber::Fiber() {
  // 主协程
  m_state = EXEC;
  SetThis(this);
  if (getcontext(&m_ctx)) {
    SOLAR_ASSERT2(false, "getcontext");
  }
  ++s_fiber_count;
  SOLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber";
}

Fiber::Fiber(std::function<void()> cb, size_t stackSize, bool use_caller)
    : m_id(++s_fiber_id), m_cb{cb} {
  ++s_fiber_count;
  m_stacksize = stackSize > 0 ? stackSize : g_fiber_stack_size->getValue();
  m_stack = StackAllocator::Alloc(m_stacksize);
  if (getcontext(&m_ctx)) {
    SOLAR_ASSERT2(false, "getcontext");
  }
  m_ctx.uc_link = nullptr;
  m_ctx.uc_stack.ss_sp = m_stack;
  m_ctx.uc_stack.ss_size = m_stacksize;
  if (!use_caller) {
    makecontext(&m_ctx, &Fiber::MainFunc, 0);
  } else {
    makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
  }
  SOLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber id=" << m_id;
}

Fiber::~Fiber() {
  --s_fiber_count;
  // 子协程才有 m_stack
  if (m_stack) {
    SOLAR_ASSERT(m_state == TERM || m_state == INIT || m_state == EXCEPT);
    StackAllocator::Dealloc(m_stack, m_stacksize);
  } else {
    SOLAR_ASSERT(nullptr == m_cb);
    SOLAR_ASSERT(m_state == EXEC);
    Fiber *cur = t_fiber;
    if (cur == this) {
      SetThis(nullptr);
    }
  }
  SOLAR_LOG_DEBUG(g_logger) << "Fiber::~Fiber id=" << m_id;
}

void Fiber::reset(std::function<void()> cb) {
  SOLAR_ASSERT(m_stack);
  SOLAR_ASSERT(m_state == TERM || m_state == INIT || m_state == EXCEPT);
  m_cb = cb;
  if (getcontext(&m_ctx)) {
    SOLAR_ASSERT2(false, "getcontext");
  }
  m_ctx.uc_link = nullptr;
  m_ctx.uc_stack.ss_sp = m_stack;
  m_ctx.uc_stack.ss_size = m_stacksize;
  makecontext(&m_ctx, &Fiber::MainFunc, 0);
  m_state = INIT;
}

void Fiber::call() {
  SetThis(this);
  m_state = EXEC;
  if (swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {
    SOLAR_ASSERT2(false, "swapcontext");
  }
}

void Fiber::back() {
  SetThis(t_threadFiber.get());
  if (swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
    SOLAR_ASSERT2(false, "swapcontext");
  }
}
void Fiber::swapIn() {
  SetThis(this);
  SOLAR_ASSERT(m_state != EXEC);
  m_state = EXEC;
  if (swapcontext(&Scheduler::GetMainFiber()->m_ctx, &m_ctx)) {
    SOLAR_ASSERT2(false, "swapcontext");
  }
}

void Fiber::swapOut() {
  // 防止自己切换到自己，MainFunc 直接运行到底了
  SetThis(Scheduler::GetMainFiber());
  if (swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx)) {
    SOLAR_ASSERT2(false, "swapcontext");
  }
}
void Fiber::SetThis(Fiber *f) { t_fiber = f; }

Fiber::ptr Fiber::GetThis() {
  if (t_fiber) {
    return t_fiber->shared_from_this();
  }
  // 没有当前协程，因此也没有主协程, 创建主协程
  Fiber::ptr mainFiber = std::shared_ptr<Fiber>(new Fiber);
  SOLAR_ASSERT(t_fiber == mainFiber.get());
  t_threadFiber = mainFiber;
  return t_fiber->shared_from_this();
}

void Fiber::YeildToRead() {
  Fiber::ptr cur = GetThis();
  cur->m_state = READY;
  cur->swapOut();
}

void Fiber::YeildToHold() {
  Fiber::ptr cur = GetThis();
  cur->m_state = HOLD;
  cur->swapOut();
}

uint64_t Fiber::TotalFibers() { return s_fiber_count; }

void Fiber::MainFunc() {
  Fiber::ptr cur = GetThis();
  SOLAR_ASSERT(cur);
  try {
    cur->m_cb();
    cur->m_cb = nullptr; // 将 function 析构，取消其对各种资源（比如 function
                         // 中的智能指针）的所有权
    cur->m_state = TERM;
  } catch (std::exception &ex) {
    cur->m_state = EXCEPT;
    SOLAR_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
                              << " id = " << GetFiberID() << std::endl
                              << BacktraceToString();
  } catch (...) {
    cur->m_state = EXCEPT;
    SOLAR_LOG_ERROR(g_logger) << "Fiber Excpet"
                              << " id = " << GetFiberID() << std::endl
                              << BacktraceToString();
  }
  auto raw_ptr = cur.get();
  cur.reset();
  raw_ptr->swapOut();
  SOLAR_ASSERT2(false, "nerver reach");
  SOLAR_LOG_ERROR(g_logger) << " id = " << GetFiberID() << std::endl
                            << BacktraceToString();
}

void Fiber::CallerMainFunc() {
  Fiber::ptr cur = GetThis();
  SOLAR_ASSERT(cur);
  try {
    cur->m_cb();
    cur->m_cb = nullptr; // 将 function 析构，取消其对各种资源（比如 function
                         // 中的智能指针）的所有权
    cur->m_state = TERM;
  } catch (std::exception &ex) {
    cur->m_state = EXCEPT;
    SOLAR_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what()
                              << " id = " << GetFiberID() << std::endl
                              << BacktraceToString();
  } catch (...) {
    cur->m_state = EXCEPT;
    SOLAR_LOG_ERROR(g_logger) << "Fiber Excpet"
                              << " id = " << GetFiberID() << std::endl
                              << BacktraceToString();
  }
  auto raw_ptr = cur.get();
  cur.reset();
  raw_ptr->back();
  SOLAR_ASSERT2(false, "nerver reach");
  SOLAR_LOG_ERROR(g_logger) << " id = " << GetFiberID() << std::endl
                            << BacktraceToString();
}

uint64_t Fiber::GetFiberID() {
  if (t_fiber) {
    return t_fiber->getID();
  }
  return 0;
}

} // namespace solar