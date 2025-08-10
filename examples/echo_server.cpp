//
// Created by guo on 2025/8/8.
//

#include "echo_server.h"

#include <iostream>

#include "Log/Log.h"
#include "Log/Logger.h"

static solar::Logger::ptr g_logger = SOLAR_LOG_ROOT();

EchoServer::EchoServer(int type)
    :m_type{type}
{
}

void EchoServer::handleClient(solar::Socket::ptr client) {
    SOLAR_LOG_INFO(g_logger) << "handleClient" << *client;
    solar::ByteArray::ptr ba = std::make_shared<solar::ByteArray>();
    while (true) {
        ba->clear();
        std::vector<iovec> iovs{};
        ba->getWriteBuffers(iovs, 1024);
        int rt = client->recv(&iovs[0], iovs.size());
        if (rt == 0) {
            SOLAR_LOG_INFO(g_logger) << "client close: " << *client;
            break;
        } else if (rt < 0) {
            SOLAR_LOG_INFO(g_logger) << "client error rt=" << rt
                << " errno=" << errno << " errstr=" << strerror(errno);
            break;
        }
        ba->setPosition(ba->getPosition() + rt); // 改变 ba 的 size
        ba->setPosition(0); // 将所有内容都 toString

        if (m_type == 1) { //text
           std::cout << ba->toString();
        } else {
            std::cout << ba->toHexString();
        }
        std::cout.flush();
    }
}

static int type = 1;

void run() {
    EchoServer::ptr es = std::make_shared<EchoServer>(type);
    auto addr = solar::Address::LookUpAny("0.0.0.0:8020");
    while (!es->bind(addr)) {
        sleep(2);
    }
    es->start();
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        SOLAR_LOG_INFO(g_logger) << "used as[" << argv[0] << "-t] or [" << argv[0] << " -bj";
        return 0;
    }
    if (!strcmp(argv[1], "-b")) {
        type = 2;
    }
    solar::IOManager iom{2};
    iom.schedule(run);
}

