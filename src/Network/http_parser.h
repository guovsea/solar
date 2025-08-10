//
// Created by guo on 2025/8/4.
//

#ifndef __SOLAR_HTTP_PARSER_H__
#define __SOLAR_HTTP_PARSER_H__

#include <memory>

#include "http11_parser.h"
#include "httpclient_parser.h"

#include "Http.h"

namespace solar::http {
class HttpRequestParser {
public:
    typedef std::shared_ptr<HttpRequestParser> ptr;
    enum ErrorCode {
        Success = 0,
        InvalidMethod = 1000,
        InvalidVersion = 10001,
        InvalidField
    };
    HttpRequestParser();

    /**
     * @brief 解析 http header 时的 buffer size
     * @return 允许的 http header 的最大长度，header 超过 HttpRequestBufferSize 时视为错误
     */
    static uint64_t GetHttpRequestBufferSize();
    static uint64_t GetHttpRequestMaxBodySize();

    size_t execute(char* data, size_t len);
    int isFinished();
    bool hasError();
    void setError(int v) { m_error = v; }
    HttpRequest::ptr getData() const { return m_data; }
    uint64_t getContextLength();
private:
    http_parser m_parser;
    HttpRequest::ptr m_data;
    // 1000 : invalid method
    int m_error;
};

class HttpResponseParser {
public:
    typedef std::shared_ptr<HttpResponseParser> ptr;

    enum ErrorCode {
        Success = 0,
        InvalidVersion = 1001,
        InvalidField = 1002
    };
    HttpResponseParser();
    size_t execute(char* data, size_t len);
    int isFinished();
    void setError(int v) { m_error = v; }
    bool hasError();
    HttpResponse::ptr getData() const { return m_data; }
private:
    httpclient_parser  m_parser;
    HttpResponse::ptr m_data;
    int m_error;
};
}
#endif //__SOLAR_HTTP_PARSER_H__