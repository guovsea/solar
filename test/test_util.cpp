#include <assert.h>
#include <gtest/gtest.h>

#include "Log/Log.h"
#include "Util/Util.h"
#include "Util/macro.h"

namespace {
solar::Logger::ptr g_logger = SOLAR_LOG_ROOT();
void func() { SOLAR_ASSERT(false); }
} // namespace

TEST(TestUtil, TestBacktrace) {
    // SOLAR_LOG_INFO(g_logger) << solar::BacktraceToString(10);
}

TEST(TestUtil, TestASSERT) {
    // func();
}

TEST(TestUtil, TestASSERT2) {
    // SOLAR_ASSERT2(false, "Some message.");
}