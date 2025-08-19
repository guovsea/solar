/**
 * @file HttpConnection.h
 * @brief Http Connection：Client 对于 Server 的一次连接
 * @author guovsea
 * @email guovsea@gmail.com
 */
#ifndef __SOLAR_HTTPCONNECTION_H__
#define __SOLAR_HTTPCONNECTION_H__

#include <list>

#include "Core/Mutex.h"
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
        TIMEOUT,
        POOL_GET_CONNECTION_FAIL,
        POOL_INVALID_CONNECTION
    };
    HttpResult(int result_, HttpResponse::ptr response_, std::string error_)
        :result{ result_ }
        ,response{ response_ }
        ,error{ error_ }
    {}
    int result;
    HttpResponse::ptr response;
    std::string error;

    std::string toString() const;
};

class HttpConnection : public SocketStream {
    friend  class HttpConnectionPool;
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
private:
    uint64_t m_createTime;
};

class HttpConnectionPool {
public:
    typedef std::shared_ptr<HttpConnectionPool> ptr;
    typedef Mutex MutexType;

    HttpConnectionPool(const std::string& host, const std::string& vhost
                        ,uint32_t port, uint32_t maxSize
                        ,uint32_t maxAliveTime, uint32_t maxRequest);

    HttpConnection::ptr getConnection();

    HttpResult::ptr doGet(const std::string& url, uint64_t timeout_ms
                          , const std::map<std::string, std::string>& headers = {}
                          , const std::string& body = {});

    HttpResult::ptr doPost(const std::string& url, uint64_t timeout_ms
                            , const std::map<std::string, std::string>& headers = {}
                            , const std::string& body = {});

    HttpResult::ptr doGet(Uri::ptr uri, uint64_t timeout_ms
                            , const std::map<std::string, std::string>& headers = {}
                            , const std::string& body = {});

    HttpResult::ptr doPost(Uri::ptr uri, uint64_t timeout_ms
                            , const std::map<std::string, std::string>& headers = {}
                            , const std::string& body = {});

    HttpResult::ptr doRequest(HttpMethod method, const std::string& url, uint64_t timeout_ms
                                , const std::map<std::string, std::string>& headers = {}
                                , const std::string& body = {});

    HttpResult::ptr doRequest(HttpMethod method, Uri::ptr uri, uint64_t timeout_ms
                                , const std::map<std::string, std::string>& headers = {}
                                , const std::string& body = {});

    HttpResult::ptr doRequest(HttpRequest::ptr req, uint64_t timeout_ms);

private:
    static void ReleasePtr(HttpConnection* ptr, HttpConnectionPool* pool);

private:
    std::string m_host;
    std::string m_vhost;
    uint32_t m_port;
    //< 连接池中持续存活最大连接数，当连接数大于该值时，一些 {TODO} 连接结束后会被释放
    uint32_t m_maxSize;
    uint32_t m_maxAliveTime;
    uint32_t m_maxRequest;

    MutexType m_mutex;
    std::list<HttpConnection*> m_conns;
    std::atomic<int32_t> m_total{ 0 };
};
}


#endif //__SOLAR_HTTPCONNECTION_H__