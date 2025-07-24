#ifndef __SOLAR_UTIL_MICRO_H__
#define __SOLAR_UTIL_MICRO_H__
#include <assert.h>
#include "Log/Log.h"
#include "Util/Util.h"

#define SOLAR_ASSERT(x)                                                                                                \
    if (!(x)) {                                                                                                        \
        SOLAR_LOG_ERROR(SOLAR_LOG_ROOT()) << "ASSERTION: " #x << "\nbacktrace\n"                                       \
                                          << solar::BacktraceToString(100, 2, "    ");                                 \
        assert(x);                                                                                                     \
    }

#define SOLAR_ASSERT2(x, w)                                                                                            \
    if (!(x)) {                                                                                                        \
        SOLAR_LOG_ERROR(SOLAR_LOG_ROOT()) << "ASSERTION: " #x << "\n"                                                  \
                                          << w << "\nbacktrace\n"                                                      \
                                          << solar::BacktraceToString(100, 2, "    ");                                 \
        assert(x);                                                                                                     \
    }
#endif