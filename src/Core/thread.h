#ifndef __SOLAR_CORE_THREAD_H__
#define __SOLAR_CORE_THREAD_H__

#include <functional>
#include <memory>
#include <string>

namespace solar {
class Thread {
  public:
    typedef std::shared_ptr<Thread> ptr;
    Thread(std::function<void()> cb, const std::string &name);
    ~Thread();

    pid_t getId() const { return m_id; }
    std::string getName() const { return m_name; }

    void join();

    // 获取当前线程的属性
    static Thread *GetThis();
    static std::string GetName();
    static void SetName(const std::string &name);

    // 线程的工作函数
    static void* run(void* arg);
  private:
    Thread(const Thread &) = delete;
    Thread(const Thread &&) = delete;
    Thread &operator=(const Thread &) = delete;
    pid_t m_id = -1;
    pthread_t m_thread = 0;
    std::function<void()> m_cb;
    std::string m_name;
};
} // namespace solar
#endif /* __SOLAR_CORE_THREAD_H__ */
