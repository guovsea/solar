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
    ~HttpConnection();
    HttpResponse::ptr recvResponse();
    int sendRequest(HttpRequest::ptr rsp);
private:
    uint64_t m_createTime;
    uint64_t m_request; //< 已经发送到请求数
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
     /**
     * @brief getConnection 返回的 HttpConnection::ptr 的自定义删除函数。
     * 当 connection 关闭时, connection 数量大于 m_maxSize 时 delete ptr
     * 否则，将 ptr 重新添加会 pool
     * @param ptr
     * @param pool
     */
    static void ReleasePtr(HttpConnection* ptr, HttpConnectionPool* pool);

private:
    std::string m_host;
    std::string m_vhost;
    uint32_t m_port;
    uint32_t m_maxSize;  //< 连接池中始终存在的最大连接数量，连接数量大于该值时，
                         //  getConnection 也能成功，但是 get 出的 Connection 会被析构
    uint32_t m_maxAliveTime;
    uint32_t m_maxRequest; //< 每个连接发送到最大请求数

    MutexType m_mutex;
    std::list<HttpConnection*> m_conns;
    std::atomic<int32_t> m_total{ 0 };  //< 通过该 Pool 申请的总连接数（不一定在 pool 中，可能已经被取走）
};
}


#endif //__SOLAR_HTTPCONNECTION_H__