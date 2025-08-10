//
// Created by guo on 2025/8/10.
//

#ifndef __SOLAR_HTTPSESSION_H__
#define __SOLAR_HTTPSESSION_H__
#include "Network/SocketStream.h"
#include "Network/Http.h"

namespace solar::http {
class HttpSession : public SocketStream {
public:
    typedef std::shared_ptr<HttpSession> ptr;
    HttpSession(Socket::ptr sock, bool owner = true);
    ~HttpSession();
    HttpRequest::ptr recvRequest();
    int sendResponse(HttpResponse::ptr rsp);
};
}


#endif //__SOLAR_HTTPSESSION_H__