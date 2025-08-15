/**
 * @file HttpSession.h
 * @brief Http Session : 表示 Server 处理一个 Client 的整个过程的逻辑概念
 * @author guovsea
 * @email guovsea@gmail.com
 */

#ifndef __SOLAR_HTTPSESSION_H__
#define __SOLAR_HTTPSESSION_H__
#include "Network/SocketStream.h"
#include "Network/Http.h"

namespace solar::http {
class HttpSession : public SocketStream {
public:
    typedef std::shared_ptr<HttpSession> ptr;
    HttpSession(Socket::ptr sock, bool owner = true);
    HttpRequest::ptr recvRequest();
    int sendResponse(HttpResponse::ptr rsp);
};
}


#endif //__SOLAR_HTTPSESSION_H__