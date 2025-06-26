#ifndef __SOLAR_UTIL_UTIL_H__
#define __SOLAR_UTIL_UTIL_H__

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

namespace solar {

pid_t GetThreadId();
uint32_t GetFiberId();

} // namespace solar

#endif /* __SOLAR_UTIL_UTIL_H__ */
