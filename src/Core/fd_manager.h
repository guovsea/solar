#ifndef __SOLAR_CORE_FD_MANAGER_H
#define __SOLAR_CORE_FD_MANAGER_H
#include "Core/Mutex.h"
#include "Util/Singleton.h"
#include <memory>
#include <vector>

namespace solar {
class IOManager;
class FdCtx : public std::enable_shared_from_this<FdCtx> {
public:
  typedef std::shared_ptr<FdCtx> ptr;
  FdCtx(int fd);
  ~FdCtx();

  bool init();
  bool isInit() const { return m_isInit; }
  bool isSocket() const { return m_isSocket; }
  bool isClosed() const { return m_isClosed; }
  bool close();

  void setUserNonblock(bool v) { m_userNonblock = v; }
  bool getUserNonblock() const { return m_userNonblock; }

  void setSysNonblock(bool v) { m_sysNonblock = v; }
  bool getSysNonblock() const { return m_sysNonblock; }

  void setTimeout(int type, uint64_t v);
  uint64_t getTimeout(int type);

private:
  bool m_isInit{true};
  bool m_isSocket{true};
  bool m_sysNonblock{true};
  bool m_userNonblock{true};
  bool m_isClosed{true};
  int m_fd;

  uint64_t m_recvTimeout;
  uint64_t m_sendTimeout;
  IOManager *m_iomanager;
};

class FdManager {
public:
  typedef RWMutex RWMutexType;
  FdManager();
  FdCtx::ptr get(int fd, bool auto_create = false);
  void del(int fd);

private:
  RWMutexType m_mutex;
  std::vector<FdCtx::ptr> m_datas;
};

typedef Singleton<FdManager> FdMgr;
} // namespace solar

#endif