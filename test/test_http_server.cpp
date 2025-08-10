//
// Created by guo on 2025/8/10.
//
#include <gtest/gtest.h>

#include "Log/Log.h"
#include "Network/HttpServer.h"

static solar::Logger::ptr g_logger = SOLAR_LOG_ROOT();


void run() {
    solar::http::HttpServer::ptr server = std::make_shared<solar::http::HttpServer>();
    solar::Address::ptr addr = solar::Address::LookUpAnyIpAddress("0.0.0.0:8020");
    while (!server->bind(addr)) {
        sleep(2);
    }
    server->start();
}

TEST(test_http_server, server) {
    solar::IOManager iom{2};
    iom.schedule(run);
}

