#include "Core/Thread.h"
#include "Log/Log.h"
using namespace solar;

namespace {
thread_local Thread *t_thread = nullptr;
thread_local std::string t_thread_name = "UNKNOW";
} // namespace
namespace solar {

Thread::Thread(std::function<void()> cb, const std::string &name)
    : m_cb(cb), m_name(name), m_semaphore(0) /* 信号量初始值为0 */ {
  if (name.empty()) {
    m_name = "UNKNOW";
  }
  int rt = pthread_create(&m_thread, nullptr, &Thread::Run, this);
  if (rt) {
    SOLAR_LOG_ERROR(SOLAR_LOG_ROOT())
        << "pthread_create thread fail, rt = " << rt << "name = " << name;
  }
  m_semaphore.wait(); // 创建 Thread 对象的线程阻塞等待子线程创建成功并设置
                      // Thread 对象的 ID
  // 比如创建 Thread 对象 t 之后立马想 t.GetId()
  // 此时 m_id 还没准备好，要在子线程运行时才准备好
}
Thread::~Thread() {
  if (m_thread) {
    pthread_detach(m_thread);
  }
}
void Thread::join() {
  if (m_thread) {
    int rt = pthread_join(m_thread, nullptr);
    if (rt) {
      SOLAR_LOG_ERROR(SOLAR_LOG_ROOT())
          << "pthrad_jon thread fail, rt = " << rt << " name = " << m_name;
    }
    m_thread = 0;
  }
}

void *Thread::Run(void *arg) {
  Thread *thread = (Thread *)arg;
  t_thread = thread;
  t_thread_name = thread->m_name;
  thread->m_id = solar::GetThreadId();
  // 修改通过 ps -T -p 打印出来的线程的名称, 方便调试
  pthread_setname_np(thread->m_thread, thread->m_name.substr(0, 15).c_str());

  thread->m_semaphore.notify(); // 线程对象中的所有数据都已经准备好了，通知创建
                                // Thread 对象的线程
  std::function<void()> cb;
  cb.swap(thread->m_cb);

  cb();
  return nullptr;
}

Thread *Thread::GetThis() { return t_thread; }

std::string Thread::GetName() { return t_thread_name; }

void Thread::SetName(const std::string &name) { t_thread_name = name; }

} // namespace solar