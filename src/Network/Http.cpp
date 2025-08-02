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
        return #name; \
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
    :m_method{HttpMethod::GET}
    ,m_status{}
    ,m_version{version}
    ,m_close{close}
{
}

std::string HttpRequest::getHeader(const std::string &key, const std::string &default_val) {
}

std::string HttpRequest::getParam(const std::string &key, const std::string &default_val) {
}

std::string HttpRequest::getCookie(const std::string &key, const std::string &default_val) {
}

void HttpRequest::setHeader(const std::string &key, const std::string &val) {
}

void HttpRequest::setParam(const std::string &key, const std::string &val) {
}

void HttpRequest::setCookie(const std::string &key, const std::string &val) {
}

void HttpRequest::delHeader(const std::string &key) {
}

void HttpRequest::delParam(const std::string &key) {
}

void HttpRequest::delCookie(const std::string &key) {
}

bool HttpRequest::hasHeader(const std::string &key, std::string *val) {
}

bool HttpRequest::hasParam(const std::string &key, std::string *val) {
}

bool HttpRequest::hasCookie(const std::string &key, std::string *val) {
}

}

