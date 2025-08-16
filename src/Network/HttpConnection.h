/**
 * @file HttpConnection.h
 * @brief Http Connection：Client 对于 Server 的一次连接
 * @author guovsea
 * @email guovsea@gmail.com
 */
#ifndef __SOLAR_HTTPCONNECTION_H__
#define __SOLAR_HTTPCONNECTION_H__

#include "Network/Uri.h"
#include "Network/SocketStream.h"
#include "Network/Http.h"


namespace solar::http {
struct HttpResult {
    typedef std::shared_ptr<HttpResult> ptr;
    enum Error{
        OK = 0,
        INVALID_URI,
        INVALID_HOST,
        CREATE_SOCKET_ERROR,
        CONNECT_FAIL,
        SEND_CLOSE_BY_PEER,
        SEND_SOCKET_ERROR,
        TIMEOUT
    };
    HttpResult(int result_, HttpResponse::ptr response_, std::string error_)
        :result{ result_ }
        ,response{ response_ }
        ,error{ error_ }
    {}
    int result;
    HttpResponse::ptr response;
    std::string error;
};

class HttpConnection : public SocketStream {
public:
    typedef std::shared_ptr<HttpConnection> ptr;

    static HttpResult::ptr DoGet(const std::string& url, uint64_t timeout_ms
                            , const std::map<std::string, std::string>& headers = {}
                            , const std::string& body = {});

    static HttpResult::ptr DoPost(const std::string& url, uint64_t timeout_ms
                            , const std::map<std::string, std::string>& headers = {}
                            , const std::string& body = {});

    static HttpResult::ptr DoGet(Uri::ptr uri, uint64_t timeout_ms
                            , const std::map<std::string, std::string>& headers = {}
                            , const std::string& body = {});

    static HttpResult::ptr DoPost(Uri::ptr uri, uint64_t timeout_ms
                            , const std::map<std::string, std::string>& headers = {}
                            , const std::string& body = {});

    static HttpResult::ptr DoRequest(HttpMethod method, const std::string& url, uint64_t timeout_ms
                                , const std::map<std::string, std::string>& headers = {}
                                , const std::string& body = {});

    static HttpResult::ptr DoRequest(HttpMethod method, Uri::ptr uri, uint64_t timeout_ms
                                , const std::map<std::string, std::string>& headers = {}
                                , const std::string& body = {});

    static HttpResult::ptr DoRequest(HttpRequest::ptr req, Uri::ptr uri, uint64_t timeout_ms);

    HttpConnection(Socket::ptr sock, bool owner = true);
    HttpResponse::ptr recvResponse();
    int sendRequest(HttpRequest::ptr rsp);
};
}


#endif //__SOLAR_HTTPCONNECTION_H__