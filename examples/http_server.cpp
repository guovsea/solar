//
// Created by guo on 2025/8/23.
//

#include "Network/HttpServer.h"
#include "Log/Log.h"

solar::Logger::ptr g_logger = SOLAR_LOG_ROOT();

void run() {
    solar::Address::ptr addr = solar::Address::LookUpAnyIpAddress("0.0.0.0:8020");
    if (!addr) {
        SOLAR_LOG_ERROR(g_logger) << "get address error";
        return;
    }
    solar::http::HttpServer::ptr http_server = std::make_shared<solar::http::HttpServer>();
    while (!http_server->bind(addr)) {
        SOLAR_LOG_ERROR(g_logger) << "bind" << *addr << " fail";
        sleep(1);
    }
    http_server->start();
}

int main(int argc, char* argv[]) {
    solar::IOManager iom{ 1 };
    iom.schedule(run);
    return 0;
}