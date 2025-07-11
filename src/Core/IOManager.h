#ifndef __SOLAR_CORE_IOMANAGER_H
#define __SOLAR_CORE_IOMANAGER_H
#include "Core/Scheduler.h"

namespace solar {
class IOManager : public Scheduler {
public:
  typedef std::shared_ptr<IOManager> ptr;
  typedef RWMutex RWMutexType;
  enum Event {
    NONE = 0x0,
    READ = 0x1,  //< EPOLLIN
    WRITE = 0x4, //< EPOLLOUT
  };
  IOManager(size_t threads = 1, bool use_caller = true,
            const std::string &name = "");
  ~IOManager();
  int addEvent(int fd, Event event, std::function<void()> cb = {});
  bool delEvent(int fd, Event event);

  // 删除，并且强制触发执行
  bool cancelEvent(int fd, Event event);
  bool cancelAll(int fd);

  static IOManager *GetThis();

protected:
  void tickle() override;

  void run();

  bool stopping() override;

  void idle() override;

  void resizeContexts(size_t size);

private:
  struct FdContext {
    typedef Mutex MutexType;
    struct EventContext {
      Scheduler *scheduler{nullptr}; //< 事件执行的 scheduler
      std::shared_ptr<Fiber> fiber;  //< 事件协程
      std::function<void()> cb;      //< 事件的回调函数
    };

    EventContext &getContext(Event event);
    void resetContext(EventContext &ctx);
    void triggerEvent(Event event);
    EventContext read;   //< 读事件
    EventContext write;  //< 写事件
    int fd{0};           //< 事件关联的句柄
    Event events = NONE; //< 已经注册的事件
    MutexType mutex;
  };
  int m_epfd{0};
  int m_tickleFds[2]{0}; // 管道
  std::atomic<int> m_pendingEventCount{0};
  RWMutexType m_mutex;
  std::vector<FdContext *> m_fdContexts;
};

} // namespace solar

#endif