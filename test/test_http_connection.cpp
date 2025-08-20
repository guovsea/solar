#include <gtest/gtest.h>

#include "Core/IOManager.h"
#include "Log/Log.h"
#include "Network/HttpConnection.h"

static solar::Logger::ptr g_logger = SOLAR_LOG_ROOT();


TEST(test_http_connection, with_content_length) {
    GTEST_SKIP();
    // 利用 www.httpbin.org 进行测试
    solar::Address::ptr addr = solar::Address::LookUpAnyIpAddress("www.httpbin.org:80");
    EXPECT_TRUE(addr) << "get addr error";

    solar::Socket::ptr sock = solar::Socket::CreateTCP(addr);
    bool rt = sock->connect(addr);
    EXPECT_TRUE(rt);
    solar::http::HttpConnection::ptr conn = std::make_shared<solar::http::HttpConnection>(sock);
    solar::http::HttpRequest::ptr req = std::make_shared<solar::http::HttpRequest>();
    req->setPath("/get");
    // HTTP/1.1 必须有 host
    req->setHeader("host", "www.httpbin.org");
    conn->sendRequest(req);
    auto rsp = conn->recvResponse();
    EXPECT_TRUE(rsp);
    std::string s = rsp->toString();
    if (rsp) {
        SOLAR_LOG_INFO(g_logger) << "rsp:\n" << *rsp;
    }
}



TEST(test_http_connection, with_chunked) {
    GTEST_SKIP();
    solar::Address::ptr addr = solar::Address::LookUpAnyIpAddress("www.httpbin.org:80");
    EXPECT_TRUE(addr) << "get addr error";
    solar::Socket::ptr sock = solar::Socket::CreateTCP(addr);
    bool rt = sock->connect(addr);
    EXPECT_TRUE(rt);
    solar::http::HttpConnection::ptr conn = std::make_shared<solar::http::HttpConnection>(sock);
    solar::http::HttpRequest::ptr req = std::make_shared<solar::http::HttpRequest>();
    req->setPath("/stream/5");
    // HTTP/1.1 必须有 host
    req->setHeader("host", "www.httpbin.org");
    conn->sendRequest(req);
    auto rsp = conn->recvResponse();
    EXPECT_TRUE(rsp);
    std::string s = rsp->toString();
    if (rsp) {
        SOLAR_LOG_INFO(g_logger) << "rsp:\n" << *rsp;
    }
}

TEST(test_http_connection, do_request) {
    GTEST_SKIP();
    solar::http::HttpResult::ptr rt = solar::http::HttpConnection::DoGet(
        "http://www.baidu.com", 300
        // "http://www.httpbin.org", 300
        );
    EXPECT_TRUE(rt);
    solar::http::HttpResponse::ptr rsp = rt->response;
    std::stringstream ss;
    ss << "result=" << rt->result << " error= " << rt->error
        << " rsp=" << (rsp ? rsp->toString() : "");
    std::string s = ss.str();
    SOLAR_LOG_INFO(g_logger)  << s;
}

void test_pool() {
    solar::http::HttpConnectionPool::ptr pool = std::make_shared<solar::http::HttpConnectionPool>(
        "www.github.com", "", 80, 10, 1000 * 20, 5);
    solar::IOManager::GetThis()->addTimer(1000, [pool]() {
        solar::http::HttpResult::ptr rt = pool->doGet("/", 300);
        SOLAR_LOG_INFO(g_logger)  << rt->toString();
    }, true);
}

TEST(test_http_connection, test_pool) {
    solar::IOManager iom{2};
    iom.schedule(test_pool);
}