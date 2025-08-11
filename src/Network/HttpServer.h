//
// Created by guo on 2025/8/10.
//

#ifndef __SOLAR_HTTPSERVER_H__
#define __SOLAR_HTTPSERVER_H__
#include "Servlet.h"
#include "Network/TcpServer.h"
#include "Network/HttpSession.h"

namespace solar::http {
class HttpServer : public TcpServer {
public:
    typedef std::shared_ptr<HttpServer> ptr;
    HttpServer(bool keepAlive = false
            , IOManager* worker = IOManager::GetThis(), IOManager* accept_worker = IOManager::GetThis());
    ServletDispatch::ptr getServletDispatch() const { return m_dispatch; }
    void setServletDispatch(ServletDispatch::ptr v) { m_dispatch = v; };
protected:
    void handleClient(Socket::ptr client) override;
private:
    bool m_isKeepAlive;
    ServletDispatch::ptr m_dispatch;
};
}


#endif //__SOLAR_HTTPSERVER_H__