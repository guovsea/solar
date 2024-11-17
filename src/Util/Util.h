#ifndef __SOLAR_UTIL_UTIL_H__
#define __SOLAR_UTIL_UTIL_H__

#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <stdint.h>

namespace solar {

pid_t GetThreadId();
uint32_t GetFiberId();

}

#endif /* __SOLAR_UTIL_UTIL_H__ */
