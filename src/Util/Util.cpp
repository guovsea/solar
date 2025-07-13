#include "Util/Util.h"
#include "Core/Fiber.h"
#include "Log/Log.h"
#include "Util.h"
#include <execinfo.h>
#include <sys/time.h>

namespace {
solar::Logger::ptr g_logger = SOLAR_LOG_NAME("system");
}
namespace solar {

pid_t GetThreadId() { return syscall(SYS_gettid); }

uint32_t GetFiberId() { return solar::Fiber::GetFiberID(); }

void Backtrace(std::vector<std::string> &bt, int size, int skip) {
  // 协程栈很小，不能在协程栈上创建大对象
  void **array = (void **)malloc(sizeof(void *) * size);
  size_t cnt = ::backtrace(array, size);
  char **strs = ::backtrace_symbols(array, cnt);
  if (strs == nullptr) {
    SOLAR_LOG_ERROR(g_logger) << "backtrace_symbols error";
  }
  for (size_t i = skip; i < cnt; ++i) {
    bt.push_back(std::string(strs[i]));
  }
  free(strs);
  free(array);
}

std::string BacktraceToString(int size, int skip, const std::string &prefix) {
  std::vector<std::string> bt;
  Backtrace(bt, size, skip);
  std::stringstream ss;
  for (const std::string &s : bt) {
    ss << prefix << s << std::endl;
  }
  return ss.str();
}

uint64_t GetCurrentMS() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return tv.tv_sec * 1000ul + tv.tv_usec / 1000;
}
uint64_t GetCurrentUS() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return tv.tv_sec * 1000'000ul + tv.tv_usec;
}
} // namespace solar
