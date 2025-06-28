#include <Log/Log.h>
#include <gtest/gtest.h>
#include <thread>

#include "Core/Mutex.h"
#include "Core/Semaphore.h"
#include "Core/Thread.h"

solar::Logger::ptr g_logger = SOLAR_LOG_ROOT();
void work1() {
  return; // 避免在单元测试时打印太多信息, TODO 使用测试断言，而不是肉眼观察
  SOLAR_LOG_INFO(g_logger) << "name = " << solar::Thread::GetName()
                           << " this.name = "
                           << solar::Thread::GetThis()->getName()
                           << " id = " << solar::GetThreadId()
                           << " this.id =" << solar::Thread::GetThis()->getId();
  // ps -p -T 查看线程信息
  // using namespace std::literals; // 防止线程退出太快，终端上看不到线程信息
  // std::this_thread::sleep_for(1min);
}

TEST(TestCore, TestThread) {
  SOLAR_LOG_INFO(g_logger) << "main thread begin";
  std::vector<solar::Thread::ptr> thrs;
  for (int i = 0; i < 5; ++i) {
    solar::Thread::ptr pThr(
        new solar::Thread(work1, "solar_thread_" + std::to_string(i)));
    thrs.push_back(pThr);
  }

  for (int i = 0; i < 5; ++i) {
    thrs[i]->join();
  }
  SOLAR_LOG_INFO(g_logger) << "main thread end";
}

static int g_cnt1 = 0;
solar::Semaphore g_sem{1};

void work2() {
  g_sem.wait();
  for (int i = 0; i < 1000'000; ++i) {
    g_cnt1++;
  }
  g_sem.notify();
}

TEST(TestCore, TestThreadAndSemaphore) {
  std::vector<solar::Thread::ptr> thrs;
  for (int i = 0; i < 5; ++i) {
    solar::Thread::ptr pThr(
        new solar::Thread(work2, "worker_" + std::to_string(i)));
    thrs.push_back(pThr);
  }

  for (int i = 0; i < 5; ++i) {
    thrs[i]->join();
  }
  EXPECT_EQ(g_cnt1, 5 * 1000'000);
}

static int g_cnt2 = 0;
solar::RWMutex g_rw_mutex;

void work3() {
  g_rw_mutex.wrlock();
  for (int i = 0; i < 1000'000; ++i) {
    g_cnt2++;
  }
  g_rw_mutex.unlock();
}

TEST(TestCore, TestRwLock) {
  std::vector<solar::Thread::ptr> thrs;
  for (int i = 0; i < 5; ++i) {
    solar::Thread::ptr pThr(
        new solar::Thread(work3, "worker_" + std::to_string(i)));
    thrs.push_back(pThr);
  }

  for (int i = 0; i < 5; ++i) {
    thrs[i]->join();
  }
  EXPECT_EQ(g_cnt2, 5 * 1000'000);
}

static int g_cnt3 = 0;

void work4() {
  solar::RWMutex::WriteScopedLock lock(g_rw_mutex);
  for (int i = 0; i < 1000'000; ++i) {
    g_cnt3++;
  }
}

TEST(TestCore, TestRWScopedMutexLock) {
  std::vector<solar::Thread::ptr> thrs;
  for (int i = 0; i < 5; ++i) {
    solar::Thread::ptr pThr(
        new solar::Thread(work4, "worker_" + std::to_string(i)));
    thrs.push_back(pThr);
  }

  for (int i = 0; i < 5; ++i) {
    thrs[i]->join();
  }
  EXPECT_EQ(g_cnt3, 5 * 1000'000);
}

static int g_cnt4 = 0;
solar::Mutex g_mutex;
void work5() {
  solar::Mutex::ScopedLock lock(g_mutex);
  for (int i = 0; i < 1000'000; ++i) {
    g_cnt4++;
  }
}

TEST(TestCore, TestMutexLock) {
  std::vector<solar::Thread::ptr> thrs;
  for (int i = 0; i < 5; ++i) {
    solar::Thread::ptr pThr(
        new solar::Thread(work5, "worker_" + std::to_string(i)));
    thrs.push_back(pThr);
  }

  for (int i = 0; i < 5; ++i) {
    thrs[i]->join();
  }
  EXPECT_EQ(g_cnt4, 5 * 1000'000);
}

void work6() {
  return; // 避免在单元测试时打印太多信息, TODO 使用测试断言，而不是肉眼观察
  for (int i = 0; i < 10000; ++i) {
    SOLAR_LOG_INFO(g_logger)
        << "name = " << solar::Thread::GetName()
        << " this.name = " << solar::Thread::GetThis()->getName()
        << " id = " << solar::GetThreadId()
        << " this.id = " << solar::Thread::GetThis()->getId();
  }
}

TEST(TestCore, TestLoggerInMutiThread) {

  std::vector<solar::Thread::ptr> thrs;
  for (int i = 0; i < 5; ++i) {
    solar::Thread::ptr pThr(
        new solar::Thread(work6, "logger_thread_" + std::to_string(i)));
    thrs.push_back(pThr);
  }

  for (int i = 0; i < 5; ++i) {
    thrs[i]->join();
  }
}