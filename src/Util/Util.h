#ifndef __SOLAR_UTIL_UTIL_H__
#define __SOLAR_UTIL_UTIL_H__

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

namespace solar {

pid_t GetThreadId();
uint32_t GetFiberId();

void Backtrace(std::vector<std::string> &bt, int size, int skip = 1);
std::string BacktraceToString(int size = 64, int skip = 2,
                              const std::string &prefx = {});

uint64_t GetCurrentMS();
uint64_t GetCurrentUS();
} // namespace solar

#endif /* __SOLAR_UTIL_UTIL_H__ */
