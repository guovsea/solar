/**
 * @file HttpConnection.h
 * @brief Http Connection：Client 对于 Server 的一次连接
 * @author guovsea
 * @email guovsea@gmail.com
 */
#ifndef __SOLAR_HTTPCONNECTION_H__
#define __SOLAR_HTTPCONNECTION_H__

#include "Network/SocketStream.h"
#include "Network/Http.h"

namespace solar::http {
class HttpConnection : public SocketStream {
public:
    typedef std::shared_ptr<HttpConnection> ptr;
    HttpConnection(Socket::ptr sock, bool owner = true);
    HttpResponse::ptr recvResponse();
    int sendRequest(HttpRequest::ptr rsp);
};
}


#endif //__SOLAR_HTTPCONNECTION_H__