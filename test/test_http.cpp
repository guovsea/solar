//
// Created by guo on 2025/8/3.
//

#include <gtest/gtest.h>
#include "Log/Log.h"
#include "Network/Http.h"
#include <iostream>
#include <sstream>

static solar::Logger::ptr g_logger = SOLAR_LOG_NAME("root");


TEST(TestHttp, test_request) {
    solar::http::HttpRequest::ptr req{new solar::http::HttpRequest}; req->setHeader("host", "www.baidu.com");
    req->setBody("hello world");
    std::stringstream ss;
    req->dump(ss);
    std::string str = ss.str();
    SOLAR_LOG_INFO(g_logger) << str;
}

TEST(TestHttp, test_rsponse) {
    solar::http::HttpResponse::ptr req{new solar::http::HttpResponse};
    req->setHeader("X-X", "guo");
    req->setBody("hello world");
    req->setStatus(solar::http::HttpStatus::BAD_REQUEST);
    std::stringstream ss;
    req->dump(ss);
    std::string str = ss.str();
    SOLAR_LOG_INFO(g_logger) << str;
}
