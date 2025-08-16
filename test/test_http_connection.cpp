#include <gtest/gtest.h>

#include "Log/Log.h"
#include "Network/HttpConnection.h"

static solar::Logger::ptr g_logger = SOLAR_LOG_ROOT();


TEST(test_http_connection, with_content_length) {
    // 利用 www.httpbin.org 进行测试
    solar::Address::ptr addr = solar::Address::LookUpAnyIpAddress("www.httpbin.org:80");
    EXPECT_TRUE(addr) << "get addr error";

    solar::Socket::ptr sock = solar::Socket::CreateTCP(addr);
    bool rt = sock->connect(addr);
    EXPECT_TRUE(rt);
    solar::http::HttpConnection::ptr conn = std::make_shared<solar::http::HttpConnection>(sock);
    solar::http::HttpRequest::ptr req = std::make_shared<solar::http::HttpRequest>();
    req->setPath("/get");
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
    solar::Address::ptr addr = solar::Address::LookUpAnyIpAddress("www.httpbin.org:80");
    EXPECT_TRUE(addr) << "get addr error";
    solar::Socket::ptr sock = solar::Socket::CreateTCP(addr);
    bool rt = sock->connect(addr);
    EXPECT_TRUE(rt);
    solar::http::HttpConnection::ptr conn = std::make_shared<solar::http::HttpConnection>(sock);
    solar::http::HttpRequest::ptr req = std::make_shared<solar::http::HttpRequest>();
    req->setPath("/stream/5");
    req->setHeader("host", "www.httpbin.org");
    conn->sendRequest(req);
    auto rsp = conn->recvResponse();
    EXPECT_TRUE(rsp);
    std::string s = rsp->toString();
    if (rsp) {
        SOLAR_LOG_INFO(g_logger) << "rsp:\n" << *rsp;
    }
}


