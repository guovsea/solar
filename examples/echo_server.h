//
// Created by guo on 2025/8/8.
//

#ifndef __SOLAR_ECHO_SERVER_H__
#define __SOLAR_ECHO_SERVER_H__

#include "Network/TcpServer.h"
#include "Network/ByteArray.h"

class EchoServer : public solar:: TcpServer {
public:
    EchoServer(int type);
    void handleClient(solar::Socket::ptr client) override;
private:
    int m_type; // 0 二进制，1 文本
};

#endif //__SOLAR_ECHO_SERVER_H__