//
// Created by guo on 2025/8/10.
//

#include "HttpServer.h"

#include "Log/Log.h"
#include "Log/Logger.h"

namespace solar::http {

static Logger::ptr g_logger = SOLAR_LOG_NAME("system");

HttpServer::HttpServer(bool keepAlive, IOManager *worker, IOManager *accept_worker)
    :TcpServer{ worker, accept_worker }
    ,m_isKeepAlive{ keepAlive }
    ,m_dispatch{ std::make_shared<ServletDispatch>() } {

}

void HttpServer::handleClient(Socket::ptr client) {
    HttpSession::ptr session = std::make_shared<HttpSession>(client);
    do {
        auto req = session->recvRequest();
        if (!req) {
            SOLAR_LOG_WARN(g_logger) << "recv http request fail, errno=" << errno
                << " errstr=" << strerror(errno) << " client: " << *client;
            break;
        }
        HttpResponse::ptr rsp = std::make_shared<HttpResponse>(req->getVersion(), req->isClose() || !m_isKeepAlive);
        m_dispatch->handle(req, rsp, session);
        session->sendResponse(rsp);
    } while (m_isKeepAlive);
    session->close();
}
}
