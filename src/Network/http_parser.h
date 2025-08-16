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

    /**
     * @brief 解析 http 报文。每次执行时会将未处理数据向前移动到 data 所指的位置
     * 解析完 header 之后会将 body copy 到 data 的起始位置
     * @param data 起始位置
     * @param len 数据的长度
     * @parm chunked 是否使用了分块传输编码
     * @return 已解析的字节数，data 的剩余有效数据为 len - v;
     */
    size_t execute(char* data, size_t len, bool chunked);
    int isFinished();
    bool hasError();
    void setError(int v) { m_error = v; }
    HttpRequest::ptr getData() const { return m_data; }
    uint64_t getContextLength();
    const http_parser& getParser() const { return m_parser; }
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

    static uint64_t GetHttpResponseBufferSize();
    static uint64_t GetHttpResponseMaxBodySize();

    size_t execute(char* data, size_t len, bool chunked);
    int isFinished();
    void setError(int v) { m_error = v; }
    bool hasError();
    HttpResponse::ptr getData() const { return m_data; }
    uint64_t getContextLength();
    const httpclient_parser& getParser() const { return m_parser; }
private:
    httpclient_parser  m_parser;
    HttpResponse::ptr m_data;
    int m_error;
};
}
#endif //__SOLAR_HTTP_PARSER_H__