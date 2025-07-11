#include "Core/IOManager.h"
#include "Log/Log.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

namespace {
solar::Logger::ptr g_logger = SOLAR_LOG_NAME("root");
}

void test_fiber() { SOLAR_LOG_INFO(g_logger) << "test_fiber"; }

TEST(TestIOManager, test_epoll_wait_and_addEvent) {
  solar::IOManager iom(2);
  iom.schedule(&test_fiber);

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  fcntl(sock, F_SETFL, O_NONBLOCK);

  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr("183.2.172.177");
  addr.sin_port = htons(80);

  iom.addEvent(sock, solar::IOManager::READ,
               []() { SOLAR_LOG_INFO(g_logger) << "read call back"; });
  iom.addEvent(sock, solar::IOManager::WRITE, [=]() {
    SOLAR_LOG_INFO(g_logger) << "write call back";
    // 将会调用 read call back
    solar::IOManager::GetThis()->cancelEvent(sock, solar::IOManager::READ);
  });
  int rt = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
}