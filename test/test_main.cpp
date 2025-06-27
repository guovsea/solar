#include <gtest/gtest.h>

#include "Log/Log.h"
#include "Util/Config.h"

// TODO 不知道为什么，在 log 模块中的任何 cpp 文件中定义的静态变量都不会被初始化
// 除非定义在被直接编译进可执行文件的 cpp 文件中
// static solar::LogIniter __logInit;

int main(int argc, char **argv) {
  solar::LogIniter __logInit;
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}