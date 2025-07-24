#ifndef __SOLAR_CORE_FIBER_H__
#define __SOLAR_CORE_FIBER_H__
#include <functional>
#include <memory>
#include <ucontext.h>

#include "Core/Thread.h"

namespace solar {
class Scheduler;
/**
 * @brief 协程对象
 *
 */
class Fiber : public std::enable_shared_from_this<Fiber> {
    // 不能在栈上创建 Fiber 对象
    friend class Scheduler;

public:
    typedef std::shared_ptr<Fiber> ptr;
    enum State { INIT, HOLD, EXEC, TERM, READY, EXCEPT };
    Fiber(std::function<void()> cb, size_t stackSize = 0, bool use_caller = false);
    ~Fiber();
    /**
     * @brief 重置协程函数并重置状态，当协程状态为 INIT，TERM
     *
     * @param cb
     */
    void reset(std::function<void()> cb);
    // 让出执行权
    /**
     * @brief 主协程切换到当前协程
     *
     */
    void swapIn();

    void call();
    void back();
    /**
     * @brief 当前协程切换到主协程
     *
     */
    void swapOut();

    uint64_t getID() const { return m_id; }

    State getState() const { return m_state; }

public:
    /**
     * @brief 设置主协程为当前协程
     *
     * @param f
     */
    static void SetThis(Fiber *f);

    static Fiber::ptr GetThis();
    /**
     * @brief 当前协程切换到主协程，并将当前协程设置为 Read 状态
     *
     */
    static void YeildToRead();
    /**
     * @brief 当前协程切换到主协程，并将设置为 Hold 状态
     *
     */
    static void YeildToHold();
    /**
     * @brief 当前进程中的总协程数
     *
     * @return uint64_t
     */
    static uint64_t TotalFibers();

    /**
     * @brief 主协程执行的函数
     *
     */
    static void MainFunc();

    static void CallerMainFunc();

    /**
     * @brief Get the Fiber ID of current fiber
     *
     * @return uint64_t
     */
    static uint64_t GetFiberID();

private:
    /**
     * @brief 主协程的构造函数
     *
     */
    Fiber();
    uint64_t m_id{0};
    uint32_t m_stacksize{0};
    State m_state{INIT};
    ucontext_t m_ctx;
    void *m_stack = nullptr;
    std::function<void()> m_cb;
};

} // namespace solar

#endif