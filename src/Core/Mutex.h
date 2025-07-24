#ifndef __SRC_CORE_MUTEX_H__
#define __SRC_CORE_MUTEX_H__

#include <atomic>
#include <pthread.h>
#include "Core/LockImpl.h"
#include "Util/Noncopyable.h"

namespace solar {

/**
 * @brief 互斥锁
 *
 */
class Mutex : public Noncopyable {
public:
    typedef ScopedLockImpl<Mutex> Lock;
    Mutex() { pthread_mutex_init(&m_mutex, nullptr); }
    ~Mutex() { pthread_mutex_destroy(&m_mutex); }
    void lock() { pthread_mutex_lock(&m_mutex); }
    void unlock() { pthread_mutex_unlock(&m_mutex); }

private:
    pthread_mutex_t m_mutex;
};

/**
 * @brief 替换某种锁，方便测试
 *
 */
class NullMutex : public Noncopyable {
public:
    typedef ScopedLockImpl<Mutex> Lock;
    NullMutex() {}
    ~NullMutex() {}
    void lock() {}
    void unlock() {}
};

class SpinLock {
public:
    typedef ScopedLockImpl<SpinLock> Lock;
    SpinLock() { pthread_spin_init(&m_mutex, 0); }
    ~SpinLock() { pthread_spin_destroy(&m_mutex); }
    void lock() { pthread_spin_lock(&m_mutex); }
    void unlock() { pthread_spin_unlock(&m_mutex); }

private:
    pthread_spinlock_t m_mutex;
};

/**
 * @brief 读写锁
 *
 */
class RWMutex : public Noncopyable {
public:
    typedef ReadScopedLockImpl<RWMutex> ReadScopedLock;
    typedef WriteScopedLockImpl<RWMutex> WriteScopedLock;

    RWMutex() { pthread_rwlock_init(&m_lock, nullptr); }
    ~RWMutex() { pthread_rwlock_destroy(&m_lock); }
    void rdlock() { pthread_rwlock_rdlock(&m_lock); }
    void wrlock() { pthread_rwlock_wrlock(&m_lock); }
    void unlock() { pthread_rwlock_unlock(&m_lock); }

private:
    pthread_rwlock_t m_lock;
};

// Compare and swap (OS)
class CASLock : public Noncopyable {
public:
    typedef ScopedLockImpl<CASLock> Lock;
    CASLock() { m_mutex.clear(); }
    ~CASLock() {}
    void lock() {
        while (std::atomic_flag_test_and_set_explicit(&m_mutex, std::memory_order_acquire))
            ;
    }
    void unlock() { std::atomic_flag_test_and_set_explicit(&m_mutex, std::memory_order_release); }

private:
    volatile std::atomic_flag m_mutex;
};

} // namespace solar

#endif /* __SRC_CORE_MUTEX_H__ */
