//
// Created by guo on 2025/8/6.
//

#include <gtest/gtest.h>
#include "Log/Log.h"
#include "Network/http_parser.h"

static solar::Logger::ptr g_logger = SOLAR_LOG_NAME("root");


const char* test_request_data = "GET / HTTP/1.1\r\n"
    "Host: www.baidu.com\r\n"
    "Content-Length: 10\r\n\r\n"
    "0123456789";

TEST(test_http_parser, test_HttpRequestParser) {
    solar::http::HttpRequestParser parser;
    std::string tmp{test_request_data};
    parser.execute(&tmp[0], tmp.size());
    EXPECT_FALSE(parser.hasError());
    EXPECT_EQ(parser.isFinished(), 1);
    std::string str = parser.getData()->toString();
    SOLAR_LOG_INFO(g_logger) << str;
}
const char* test_response_data = "HTTP/1.1 200 OK\r\n"
        "Date: Tue, 04 Jun 2019 15:43:56 GMT\r\n"
        "Server: Apache\r\n"
        "Last-Modified: Tue, 12 Jan 2010 13:48:00 GMT\r\n"
        "ETag: \"51-47cf7e6ee8400\"\r\n"
        "Accept-Ranges: bytes\r\n"
        "Content-Length: 81\r\n"
        "Cache-Control: max-age=86400\r\n"
        "Expires: Wed, 05 Jun 2019 15:43:56 GMT\r\n"
        "Connection: Close\r\n"
        "Content-Type: text/html\r\n\r\n"
        "<html>\r\n"
        "<meta http-equiv=\"refresh\" content=\"0;url=http://www.baidu.com/\">\r\n"
        "</html>\r\n";

TEST(test_http_parser, test_HttpResponseParser) {
    solar::http::HttpResponseParser parser;
    std::string tmp{test_response_data};
    parser.execute(&tmp[0], tmp.size());
    EXPECT_FALSE(parser.hasError());
    EXPECT_EQ(parser.isFinished(), 1);
    std::string str = parser.getData()->toString();
    SOLAR_LOG_INFO(g_logger) << tmp;
}
