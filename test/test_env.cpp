#include <gtest/gtest.h>
#include "Log/Log.h"
#include "Core/Env.h"

static solar::Logger::ptr g_logger = SOLAR_LOG_NAME("root");

TEST(test_env, test) {
    solar::EnvMgr::Instance()->addHelp("d", "run as daemon");
    solar::EnvMgr::Instance()->addHelp("s", "start with terminal");
    solar::EnvMgr::Instance()->addHelp("p", "print help");
        solar::EnvMgr::Instance()->printHelp();

    int argc = 4;
    char* argv[] = {(char*)"test_env", (char*)"-d", (char*)"-s", (char*)"-p"};
    bool rt = solar::EnvMgr::Instance()->init(argc, argv);
    EXPECT_TRUE(rt);
    if (rt) {
        EXPECT_TRUE(solar::EnvMgr::Instance()->has("d"));
        EXPECT_TRUE(solar::EnvMgr::Instance()->has("s"));
        EXPECT_TRUE(solar::EnvMgr::Instance()->has("p"));
        solar::EnvMgr::Instance()->del("p");
        EXPECT_FALSE(solar::EnvMgr::Instance()->has("p"));
    }
}

