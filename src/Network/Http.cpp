//
// Created by guo on 2025/8/2.
//

#include "Http.h"

namespace solar::http {
bool CaseInsensitiveLess::operator()(const std::string &lhs, const std::string &rhs) const {
    return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
}


HttpMethod StringToHttpMethod(const std::string &m) {
#define XX(num, name, string) \
if (strcmp(#string, m.c_str()) == 0) { \
return HttpMethod::name; \
}
    HTTP_METHOD_MAP(XX);
#undef XX
    return HttpMethod::INVALID_METHOD;
}

HttpMethod CharsToHttpMethod(const char *m) {
    return StringToHttpMethod(std::string{m});
}

const char * HttpMethodToString(HttpMethod m) {
    switch(m) {
#define XX(num, name, string) \
case HttpMethod::name: \
return #name;
        HTTP_METHOD_MAP(XX);
#undef XX
        case HttpMethod::INVALID_METHOD:
        return  "INVALID_METHOD";
        default:
        return "<unknown>";
    }
}


const char * HttpStatusToString(HttpStatus s) {
    switch (s) {
#define XX(num, name, string) \
case HttpStatus::name: \
return #name;
        HTTP_STATUS_MAP(XX);
#undef XX
        default:
        return "<unknown>";
    }
}

HttpRequest::HttpRequest(uint8_t version, bool close)
    :m_path{"/"}
    ,m_method{HttpMethod::GET}
    ,m_version{version}
    ,m_close{close}
{
}

std::string HttpRequest::getHeader(const std::string &key, const std::string &default_val) {
    auto it = m_headers.find(key);
    return it == m_headers.end() ? default_val : it->second;
}

std::string HttpRequest::getParam(const std::string &key, const std::string &default_val) {
    auto it = m_params.find(key);
    return it == m_params.end() ? default_val : it->second;
}

std::string HttpRequest::getCookie(const std::string &key, const std::string &default_val) {
    auto it = m_cookies.find(key);
    return it == m_cookies.end() ? default_val : it->second;
}

void HttpRequest::setHeader(const std::string &key, const std::string &val) {
    m_headers[key] = val;
}

void HttpRequest::setParam(const std::string &key, const std::string &val) {
    m_params[key] = val;
}

void HttpRequest::setCookie(const std::string &key, const std::string &val) {
    m_cookies[key] = val;
}

void HttpRequest::delHeader(const std::string &key) {
    m_headers.erase(key);
}

void HttpRequest::delParam(const std::string &key) {
    m_params.erase(key);
}

void HttpRequest::delCookie(const std::string &key) {
    m_cookies.erase(key);
}

bool HttpRequest::hasHeader(const std::string &key, std::string *val) {
    auto it = m_headers.find(key);
    if (it == m_headers.end()) {
        return false;
    }
    if (val) {
        *val = it->second;
    }
    return true;
}

bool HttpRequest::hasParam(const std::string &key, std::string *val) {
    auto it = m_params.find(key);
    if (it == m_params.end()) {
        return false;
    }
    if (val) {
        *val = it->second;
    }
    return true;
}

bool HttpRequest::hasCookie(const std::string &key, std::string *val) {
    auto it = m_cookies.find(key);
    if (it == m_cookies.end()) {
        return false;
    }
    if (val) {
        *val = it->second;
    }
    return true;
}

std::ostream& HttpRequest::dump(std::ostream &os) const {
    /* GET /uri HTTP/1.1    起始行 (Request Line)
     * Host: www.google.com 请求头 (Request Headers)
     * ...                  Body
     */
    // 1. 起始行 (Request Line)
    // Method
    os << HttpMethodToString(m_method) << " ";
    // URI
    os << m_path
       << (m_query.empty() ? "" : "?")
       << m_query
       << (m_fragment.empty() ? "" : "#")
       << m_fragment;
    // HTTP 协议版本
    os << " HTTP/" << ((uint32_t)m_version >> 4) // 取 (uint8_t)0x11 的高 4 位
        << "."
        << ((uint32_t)m_version & 0x0F) // 取 (uint8_t)0x11 的低 4 位
        << "\r\n";
    // 2. 请求头 (Request Headers)
    os << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";
    for (auto& i : m_headers) {
        if (i.first == "connection") {
            continue;
        }
        os << i.first << ":" << i.second << "\r\n";
    }
    if (!m_body.empty()) {
        os << "content-length: " << m_body.size() << "\r\n\r\n";
    }
    // 3. Body
    if (!m_body.empty()) {
        os << m_body << "\r\n";
    }
    return os;
}

std::string HttpRequest::toString() const {
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

HttpResponse::HttpResponse(uint8_t version, bool close)
    :m_status{HttpStatus::OK}
    ,m_version{version}
    ,m_close{close}
{
}

std::string HttpResponse::getHeader(const std::string &key, const std::string &default_val) {
    auto it = m_headers.find(key);
    return it == m_headers.end() ? default_val : it->second;
}

void HttpResponse::setHeader(const std::string &key, const std::string &val) {
    m_headers[key] = val;
}

void HttpResponse::delHeader(const std::string &key) {
    m_headers.erase(key);
}

std::ostream & HttpResponse::dump(std::ostream &os) const {
    // 1. 状态行
    os << " HTTP/" << ((uint32_t)m_version >> 4) // 取 (uint8_t)0x11 的高 4 位
       << "."
       << ((uint32_t)m_version & 0x0F) // 取 (uint8_t)0x11 的低 4 位
       << " "
       << (uint32_t)m_status
       << " "
       << (m_reason.empty() ? HttpStatusToString(m_status) : m_reason )
       << "\r\n";
    // 2. 响应头 (Response Headers)
    os << "connection: " << (m_close ? "close" : "keep-alive") << "\r\n";
    for (auto& i : m_headers) {
        if (i.first == "connection") {
            continue;
        }
        os << i.first << ":" << i.second << "\r\n";
    }
    if (!m_body.empty()) {
        os << "content-length: " << m_body.size() << "\r\n\r\n";
    }
    // 3. Response Body
    if (!m_body.empty()) {
        os << m_body << "\r\n";
    }
    return os;
}

std::string HttpResponse::toString() const {
    std::stringstream ss;
    dump(ss);
    return ss.str();
}

}
