//
// Created by guo on 2025/8/10.
//

#include <algorithm>
#include "Servlet.h"
#include  <fnmatch.h>

namespace solar::http {

FunctionServlet::FunctionServlet(callback cb)
    :Servlet{"FunctionServlet"}
    ,m_cb{cb}
{
}

int32_t FunctionServlet::handle(HttpRequest::ptr req, HttpResponse::ptr rsp, HttpSession::ptr session) {
    return m_cb(req, rsp, session);
}

ServletDispatch::ServletDispatch()
    :Servlet{"ServletDispatch"}
    ,m_default{ std::make_shared<NotFoundServlet>() }
{
}

int32_t ServletDispatch::handle(HttpRequest::ptr req, HttpResponse::ptr rsp, HttpSession::ptr session) {
    Servlet::ptr slt = getMatchedServlet(req->getPath());
    if (slt) {
        slt->handle(req, rsp, session);
    }
    return 0;
}

void ServletDispatch::addServlet(const std::string &uri, Servlet::ptr slt) {
    RWMutexType::WriteLock lock{ m_mutex };
    m_datas[uri] = slt;
}

void ServletDispatch::addServlet(const std::string &uri, FunctionServlet::callback cb) {
    RWMutexType::WriteLock lock{ m_mutex };
    m_datas[uri].reset(new FunctionServlet{cb});
}

void ServletDispatch::addGlobServlet(const std::string &uri, Servlet::ptr slt) {
    RWMutexType::WriteLock lock{ m_mutex };
    auto it = std::find_if(m_globs.begin(), m_globs.end(),
        [uri](auto& x){ return x.first == uri;}
        );
    if (it != m_globs.end()) {
        m_globs.erase(it);
    }
    m_globs.emplace_back(uri, slt);
}

void ServletDispatch::addGlobServlet(const std::string &uri, FunctionServlet::callback cb) {
    return addGlobServlet(uri, std::make_shared<FunctionServlet>(cb));
}

void ServletDispatch::delServlet(const std::string &uri) {
    RWMutexType::WriteLock lock{ m_mutex };
    m_datas.erase(uri);
}

void ServletDispatch::delGlobServlet(const std::string &uri) {
    RWMutexType::WriteLock lock{ m_mutex };
    m_globs.erase(std::remove_if(m_globs.begin(), m_globs.end(), [&uri](auto& x) {
        return x.first == uri;
    }), m_globs.end());

}

Servlet::ptr ServletDispatch::getServlet(const std::string &uri) {
    RWMutexType::WriteLock lock{ m_mutex };
    auto it = m_datas.find(uri);
    return it != m_datas.end() ? it->second : nullptr;
}

Servlet::ptr ServletDispatch::getGlobServlet(const std::string &uri) {
    RWMutexType::WriteLock lock{ m_mutex };
    auto it = std::find_if(m_globs.begin(), m_globs.end(),
        [uri](auto& x){ return x.first == uri;}
        );
    return it != m_globs.end() ? it->second : nullptr;
}

Servlet::ptr ServletDispatch::getMatchedServlet(const std::string &uri) {
    RWMutexType::WriteLock lock{ m_mutex };
    auto i = m_datas.find(uri);
    if (i != m_datas.end()) {
        return  i->second;
    }
    auto it = std::find_if(m_globs.begin(), m_globs.end(),
        [uri](auto& x) {
            bool b =  fnmatch(x.first.c_str(), uri.c_str(), 0) == 0;
            return b;
        });
    return it != m_globs.end() ? it->second : m_default;
}

NotFoundServlet::NotFoundServlet()
    :Servlet{ "NotFoundServlet" } {
}

int32_t NotFoundServlet::handle(HttpRequest::ptr req, HttpResponse::ptr rsp, HttpSession::ptr session) {
    static const std::string& RSP_BODY = "<html><head><title>404 Not Found"
            "</title></head><body><center><h1>404 Not Found</h1></center>"
            "<hr><center>" "solar/1.0.0" "</center></body></html>";
    rsp->setStatus(HttpStatus::NOT_FOUND);
    rsp->setHeader("Server", "solar/1.0.0");
    rsp->setHeader("Content-type", "text/html");
    rsp->setBody(RSP_BODY);
    return 0;
}
}
