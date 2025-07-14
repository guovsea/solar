/**
 * @file Hook.h
 * @brief Hooking 网络编程中常用的系统调用
 * @version 0.1
 * @date 2025-07-15
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef __SOLAR_CORE_HOOK_H__
#define __SOLAR_CORE_HOOK_H__

#include <unistd.h>

namespace solar {
bool is_hook_enable();
void set_hook_enable(bool falg);

} // namespace solar

extern "C" {

// sleep
typedef unsigned int (*sleep_fun)(unsigned int seconds);
extern sleep_fun sleep_f;

typedef int (*usleep_fun)(useconds_t usec);
extern usleep_fun usleep_f;
}

#endif