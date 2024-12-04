#include <gtest/gtest.h>
#include <Core/thread.h>
#include <Log/Log.h>
#include <thread>

solar::Logger::ptr g_logger = SOLAR_LOG_ROOT();
void work1() {
    SOLAR_LOG_INFO(g_logger) << "name = " << solar::Thread::GetName()
        << " this.name = " << solar::Thread::GetThis()->getName() <<
        " id = " << solar::GetThreadId()
        << " this.id = " << solar::Thread::GetThis()->getId();
        using namespace std::literals;
        std::this_thread::sleep_for(1min);
}
TEST(TestCore, TestThread) {
    SOLAR_LOG_INFO(g_logger) << "main thread begin";
    std::vector<solar::Thread::ptr> thrs;
    for (int i= 0; i  < 5; ++i) {
        solar::Thread::ptr pThr(new solar::Thread(work1, "solar_thread_" + std::to_string(i)));
        thrs.push_back(pThr);
    }

    for (int i = 0; i < 5; ++ i) {
        thrs[i]->join();
    }
    SOLAR_LOG_INFO(g_logger) << "main thread end";
}