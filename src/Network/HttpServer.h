//
// Created by guo on 2025/8/10.
//

#ifndef __SOLAR_HTTPSERVER_H__
#define __SOLAR_HTTPSERVER_H__
#include "Network/TcpServer.h"
#include "Network/HttpSession.h"

namespace solar::http {
class HttpServer : public TcpServer {
public:
    typedef std::shared_ptr<HttpServer> ptr;
    HttpServer(bool keepAlive = false
            , IOManager* worker = IOManager::GetThis(), IOManager* accept_worker = IOManager::GetThis());
protected:
    void handleClient(Socket::ptr client) override;
private:
    bool m_isKeepAlive;
};
}


#endif //__SOLAR_HTTPSERVER_H__