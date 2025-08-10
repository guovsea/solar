//
// Created by guo on 2025/8/4.
//

#include "http_parser.h"

#include "Log/Log.h"
#include "Log/Logger.h"
#include "Util/Config.h"

namespace solar::http {
static Logger::ptr g_logger = SOLAR_LOG_NAME("system");
static ConfigVar<uint64_t>::ptr g_http_request_buffer_size =
    solar::Config::Lookup("http.request.buffer_size"
        ,(uint64_t)4 * 1024, "http request buffer size");

static solar::ConfigVar<uint64_t>::ptr g_http_request_max_body_size =
    solar::Config::Lookup("http.request.max_body_size"
        ,(uint64_t)64 * 1024, "http request max body size");

static int s_http_request_buffer_size{0};
static int s_http_request_max_body_size{0};

struct _HttpRequestConfigIniter {
    _HttpRequestConfigIniter() {
        s_http_request_buffer_size = g_http_request_buffer_size->getValue();
        s_http_request_max_body_size = g_http_request_max_body_size->getValue();
        g_http_request_buffer_size->addListener(0x83758, []( const int& ov, const int& nv) {
            s_http_request_buffer_size = nv;
        });
        g_http_request_max_body_size->addListener(0x55755, []( const int& ov, const int& nv) {
            s_http_request_max_body_size = nv;
        });
    }
};

static _HttpRequestConfigIniter _init;
void on_request_method(void *data, const char *at, size_t length) {
    std::string str{at, length};
    HttpMethod m = StringToHttpMethod(str);
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);

    if (m == HttpMethod::INVALID_METHOD) {
        SOLAR_LOG_WARN(g_logger) << "invalid http request method :"
            << std::string(at, length);
        parser->setError(HttpRequestParser::InvalidMethod);
        return;
    }
    parser->getData()->setMethod(m);
}
void on_request_uri(void *data, const char *at, size_t length) {
}
void on_request_fragment(void *data, const char *at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    parser->getData()->setFragment(std::string{at, length});
}
void on_request_path(void *data, const char *at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    parser->getData()->setPath(std::string{at, length});
}
void on_request_query(void *data, const char *at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    parser->getData()->setQuery(std::string{at, length});
}
void on_request_version(void *data, const char *at, size_t length) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    uint8_t v = 0;
    if (strncmp(at, "HTTP/1.1", length) == 0) {
        v = 0x11;
    } else if (strncmp(at, "HTTP/1.0", length) == 0) {
        v = 0x10;
    } else {
        SOLAR_LOG_WARN(g_logger) << "invalid http request version: "
            << std::string{at, length};
        parser->setError(HttpRequestParser::InvalidVersion);
        return;
    }
    parser->getData()->setVersion(v);
}

void on_request_header_done(void *data, const char *at, size_t length) {
}

void on_request_http_field(void *data, const char *field, size_t flen, const char *value, size_t vlen) {
    HttpRequestParser* parser = static_cast<HttpRequestParser*>(data);
    if (flen == 0) {
        SOLAR_LOG_WARN(g_logger) << "invalid http request field length === 0";
        // parser->setError(HttpRequestParser::InvalidField);
    }
    parser->getData()->setHeader(std::string(field, flen), std::string(value, vlen));
}

HttpRequestParser::HttpRequestParser()
    :m_parser{}
,m_data{std::make_shared<HttpRequest>()}
,m_error{Success}
{
    http_parser_init(&m_parser);
    m_parser.request_method = on_request_method;
    m_parser.request_uri = on_request_uri;
    m_parser.fragment = on_request_fragment;
    m_parser.request_path = on_request_path;
    m_parser.query_string = on_request_query;
    m_parser.http_version = on_request_version;
    m_parser.header_done = on_request_header_done;
    m_parser.http_field = on_request_http_field;
    m_parser.data = this;
}

uint64_t HttpRequestParser::GetHttpRequestBufferSize() {
    return s_http_request_buffer_size;
}

uint64_t HttpRequestParser::GetHttpRequestMaxBodySize() {
    return s_http_request_max_body_size;
}

size_t HttpRequestParser::execute(char *data, size_t len) {
    size_t rt = http_parser_execute(&m_parser, data, len, 0);
    memmove(data, data + rt, len - rt);
    return rt;
}

int HttpRequestParser::isFinished() {
    return http_parser_finish(&m_parser);
}

bool HttpRequestParser::hasError() {
    return m_error || http_parser_has_error(&m_parser);
}

uint64_t HttpRequestParser::getContextLength() {
    return m_data->getHeaderAs<uint64_t>("content-length", 0);
}

void on_response_reason_phrase(void *data, const char *at, size_t length) {
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    parser->getData()->setReason(std::string{at, length});
}

void on_response_status_code(void *data, const char *at, size_t length) {
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    HttpStatus status = static_cast<HttpStatus>(atoi(at));
    parser->getData()->setStatus(status);
}

void on_response_chunk_size(void *data, const char *at, size_t length) {
}

void on_response_http_version(void *data, const char *at, size_t length) {
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    uint8_t v = 0;
    if (strncmp(at, "Http/1.1", length) == 0) {
        v = 0x11;
    } else if (strncmp(at, "Http/1.0", length) == 0) {
        v = 0x10;
    } else {
        SOLAR_LOG_WARN(g_logger) << "invalid http response version: "
            << std::string{at, length};
        parser->setError(HttpResponseParser::InvalidVersion);
    }
}

void on_response_query(void *data, const char *at, size_t length) {
}

void on_response_version(void *data, const char *at, size_t length) {
}

void on_response_header_done(void *data, const char *at, size_t length) {
}

void on_response_last_chunk(void *data, const char *at, size_t length) {
}

void on_response_http_field(void *data, const char *field, size_t flen, const char *value, size_t vlen) {
    HttpResponseParser* parser = static_cast<HttpResponseParser*>(data);
    if (flen == 0) {
        SOLAR_LOG_WARN(g_logger) << "invalid http request field length === 0";
        parser->setError(HttpResponseParser::InvalidField);
    }
    parser->getData()->setHeader(std::string(field, flen), std::string(value, vlen));
}

HttpResponseParser::HttpResponseParser()
    :m_parser{}
,m_data{std::make_shared<HttpResponse>()}
,m_error{Success}
{
    httpclient_parser_init(&m_parser);
    m_parser.reason_phrase = on_response_reason_phrase;
    m_parser.status_code = on_response_status_code;
    m_parser.chunk_size = on_response_chunk_size;
    m_parser.http_version = on_response_http_version;
    m_parser.http_version = on_response_version;
    m_parser.header_done = on_response_header_done;
    m_parser.last_chunk = on_response_last_chunk;
    m_parser.http_field = on_response_http_field;
    m_parser.data = this;
}

size_t HttpResponseParser::execute(char *data, size_t len) {
    size_t offset = httpclient_parser_execute(&m_parser, data, len, 0);
    memmove(data, data + offset, (len - offset));
    return offset;
}

int HttpResponseParser::isFinished() {
    return httpclient_parser_finish(&m_parser);
}

bool HttpResponseParser::hasError() {
    return m_error || httpclient_parser_has_error(&m_parser);
}

}
