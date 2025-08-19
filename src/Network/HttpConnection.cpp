//
// Created by guo on 2025/8/13.
//

#include "HttpConnection.h"

#include "http_parser.h"
#include "Log/Log.h"
#include "Util/Util.h"

namespace solar::http {

static Logger::ptr g_logger = SOLAR_LOG_NAME("system");

std::string HttpResult::toString() const {
    std::stringstream ss;
    ss << "(HttpResult result=" << result
        << " error=" << error
        << " response=" << (response ? response->toString() : "nullptr")
        << ")";
    return ss.str();
}

HttpResult::ptr HttpConnection::DoGet(const std::string &url, uint64_t timeout_ms,
                                      const std::map<std::string, std::string> &headers, const std::string &body) {
    Uri::ptr uri = Uri::Create(url);
    if (!uri) {
        return std::make_shared<HttpResult>(HttpResult::Error::INVALID_URI, nullptr, "invalid url:" + url);
    }
    return DoGet(uri, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoPost(const std::string &url, uint64_t timeout_ms,
        const std::map<std::string, std::string> &headers, const std::string &body) {
    Uri::ptr uri = Uri::Create(url);
    if (!uri) {
        return std::make_shared<HttpResult>(HttpResult::Error::INVALID_URI, nullptr, "invalid url:" + url);
    }
    return DoPost(uri, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoGet(Uri::ptr uri, uint64_t timeout_ms,
        const std::map<std::string, std::string> &headers, const std::string &body) {
    return DoRequest(HttpMethod::GET, uri, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoPost(Uri::ptr uri, uint64_t timeout_ms,
        const std::map<std::string, std::string> &headers, const std::string &body) {
    return DoRequest(HttpMethod::POST, uri, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoRequest(HttpMethod method, const std::string &url, uint64_t timeout_ms,
        const std::map<std::string, std::string> &headers, const std::string &body) {
    Uri::ptr uri = Uri::Create(url);
    if (!uri) {
        return std::make_shared<HttpResult>(HttpResult::Error::INVALID_URI, nullptr, "invalid url:" + url);
    }
    return DoRequest(method, uri, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnection::DoRequest(HttpMethod method, Uri::ptr uri, uint64_t timeout_ms,
        const std::map<std::string, std::string> &headers, const std::string &body) {
    HttpRequest::ptr req = std::make_shared<HttpRequest>();
    req->setMethod(method);
    req->setPath(uri->getPath());
    req->setQuery(uri->getQuery());
    req->setFragment(uri->getFragment());
    bool has_host{ false };
    for (auto& i : headers) {
        if (strcasecmp(i.first.c_str(), "connection") == 0) {
            if (strcasecmp(i.second.c_str(), "close") == 0) {
                req->setClose(false);
            }
        }
        if (strcasecmp(i.first.c_str(), "host") == 0) {
            has_host = !i.second.empty();
        }
        req->setHeader(i.first, i.second);
    }
    if (!has_host) {
        req->setHeader("Host", uri->getHost());
    }
    req->setBody(body);
    return DoRequest(req, uri, timeout_ms);

}

HttpResult::ptr HttpConnection::DoRequest(HttpRequest::ptr req, Uri::ptr uri, uint64_t timeout_ms) {
    Address::ptr addr = uri->createAddress();
    if (!addr) {
        return std::make_shared<HttpResult>(HttpResult::INVALID_HOST,
            nullptr, "invalid host: " + uri->getHost());
    }
    Socket::ptr sock = Socket::CreateTCP(addr);
    if (!sock) {
        return std::make_shared<HttpResult>(HttpResult::CREATE_SOCKET_ERROR,
            nullptr, "create socket fail: " + addr->toString()
                + " errno=" + std::to_string(errno) + "errstr=" + strerror(errno));
    }
    if (!sock->connect(addr)) {
        return std::make_shared<HttpResult>(HttpResult::CONNECT_FAIL,
            nullptr, "connect fail: " + addr->toString());
    }
    sock->setRecvTimeout(timeout_ms);
    HttpConnection::ptr conn = std::make_shared<HttpConnection>(sock);
    int rt = conn->sendRequest(req);
    if (rt == 0) {
        return std::make_shared<HttpResult>(HttpResult::SEND_CLOSE_BY_PEER,
            nullptr, "send request closed by peer" + addr->toString());
    }
    if (rt < 0) {
        return std::make_shared<HttpResult>(HttpResult::SEND_SOCKET_ERROR,
            nullptr, "send request socket error errno=" + std::to_string(errno)
                + " errstr=" + strerror(errno));
    }
    HttpResponse::ptr rsp = conn->recvResponse();
    if (!rsp) {
        return std::make_shared<HttpResult>(HttpResult::TIMEOUT,
            nullptr, "recv response timeout: " + addr->toString() +
                + " timeout_ms:" + std::to_string(timeout_ms));
    }
    return std::make_shared<HttpResult>(HttpResult::OK, rsp, "ok");
}

HttpConnection::HttpConnection(Socket::ptr sock, bool owner)
    :SocketStream{sock, owner} {
}

HttpResponse::ptr HttpConnection::recvResponse() {
    uint64_t buff_size = HttpResponseParser::GetHttpResponseBufferSize();
    std::vector<char> buffer(buff_size + 1);
    char* data = buffer.data();
    HttpResponseParser::ptr parser = std::make_shared<HttpResponseParser>();
    int offset = 0; // 未解析的数据的下一个位置
    while (true) {
        // 将读出的数据放到未解析的数据之后
        int len = read(data + offset, buff_size - offset);
        if (len <= 0) {
            close();
            return nullptr;
        }
        len += offset;  // 读了之后，所有未解析的数据的长度
        data[len] = '\0'; // 避免 httpclient_parser.rl:197 的断言
        size_t nparse = parser->execute(data, len);
        if (parser->hasError()) {
            close();
            return nullptr;
        }
        offset = len - nparse; // 解析之后剩余未解析数据的长度
        if (offset == buff_size) {
            // 有效数据满了，buff 中所有数据都未被解析，因此不能再接收数据
            close();
            return nullptr;
        }
        if (parser->isFinished()) {
            break;
        }
    }
    // header 解析完毕, body 将 从 data 的起始位置开始
    // offset 为未解析的数据 (body) 的长度
    auto& client_parser = parser->getParser();
    std::string body;
    if (client_parser.chunked) {
        // Transfer-Encoding: chunked : body 会分成多个 chunk 传输
        // 会有多个 chunk，需要多次解析，每个 chunk 以 chunk-length/r/n开头, 最后一个 chunk 的 chunk-length 为 0
        int len = offset;
        // 一个 chunk 的格式：chunk_header(chunk-length\r\n) + chunk_body(chunk-content\r\n)
        while (!client_parser.chunks_done) {
            // 因为是非阻塞 IO，所以要分多次读取数据并解析，实际上每次只解析了一个 chunk
            do {
                if (len == 0) {
                    int rt = read(data + len, buff_size - len);
                    if (rt <= 0) {
                        close();
                        return nullptr;
                    }
                    len += rt;
                }
                data[len] = '\0';
                size_t nparse = parser->execute(data, len, true);
                if (parser->hasError()) {
                    close();
                    return nullptr;
                }
                len -= nparse;
                if (len == buff_size) {
                    close();
                    return nullptr;
                }
            } while (!parser->isFinished());  // 该 chunk 的头部接收并解析完成

            // content_len -- chunk-length，表示 chunk_body 的长度 (不包括末尾的\r\n)
            constexpr int TAIL_CRLT_LEN = 2; // 解析下一个 chunk 时，必须跳过当前 chunk 末尾的 \r\n
            if (client_parser.content_len + TAIL_CRLT_LEN <= len) {
                body.append(data, client_parser.content_len);
                memmove(data, data + client_parser.content_len + TAIL_CRLT_LEN, len - client_parser.content_len - TAIL_CRLT_LEN);
                len -= client_parser.content_len + TAIL_CRLT_LEN;
            } else {
                body.append(data, len);
                len = 0;
                // 到本 chunk 结束，还需要读的数据
                int left = client_parser.content_len - (len - TAIL_CRLT_LEN);
                while (left > 0) {
                    int rt = read(data, left > buff_size ? buff_size : left);
                    if (rt <= 0) {
                        close();
                        return nullptr;
                    }
                    body.append(data, rt);
                    left -= rt;
                }
                len = 0;
            }
        }
    } else {
        int64_t length = parser->getContextLength(); //  header 中的 "Content-Length"
        if (length > 0) {
            body.resize(length);

            if (length <= offset) {
                memcpy(&body[0], data, length);
            } else {
                memcpy(&body[0], data, offset);
                length -= offset;
                // 根据 Content-Length，还有 length 长度的数据未到达，继续读取
                if (readFixSize(&body[offset], length) <=0) {
                    close();
                    return nullptr;
                }
            }
        }
    }
    parser->getData()->setBody(body);
    return parser->getData();
}

int HttpConnection::sendRequest(HttpRequest::ptr req) {
    std::string data = req->toString();
    return writeFixSize(data.c_str(), data.size());
}

HttpConnectionPool::HttpConnectionPool(const std::string &host, const std::string &vhost, uint32_t port,
        uint32_t maxSize, uint32_t maxAliveTime, uint32_t maxRequest)
            :m_host{ host }
            ,m_vhost{ vhost }
            ,m_port{ port }
            // ,m_maxSize{ maxSize }
            ,m_maxAliveTime{ maxAliveTime }
            ,m_maxRequest{ maxRequest }
{
}

HttpConnection::ptr HttpConnectionPool::getConnection() {
    uint64_t now_ms = GetCurrentMS();
    std::vector<HttpConnection*> invalid_conns;
    HttpConnection* ptr = nullptr;
    MutexType::Lock lock(m_mutex);
    while (!m_conns.empty()) {
        auto conn = *m_conns.begin();
        m_conns.pop_front();
        if (!conn->isConnected()) {
            invalid_conns.push_back(conn);
            continue;
        }
        if (conn->m_createTime + m_maxAliveTime > now_ms) {
            invalid_conns.push_back(conn);
            continue;
        }
        ptr = conn;
        break;
    }
    lock.unlock();
    for (auto i : invalid_conns) {
        delete i;
    }
    m_total -= invalid_conns.size();
    if (!ptr) {
        IPAddress::ptr addr = Address::LookUpAnyIpAddress(m_host);
        if (!addr) {
            SOLAR_LOG_ERROR(g_logger) << "get addr fail: " << m_host;
            return nullptr;
        }
        addr->setPort(m_port);
        Socket::ptr sock = Socket::CreateTCP(addr);
        if (!sock) {
            SOLAR_LOG_ERROR(g_logger) << "create sock fail: " << *addr;
            return nullptr;
        }
        if (!sock->connect(addr)) {
            SOLAR_LOG_ERROR(g_logger) << "sock connet fail: " << *addr;
            return nullptr;
        }
        ptr = new HttpConnection(sock);
        ++m_total;
    }
    return HttpConnection::ptr(ptr, [this](HttpConnection* ptr) { ReleasePtr(ptr, this); });
}

HttpResult::ptr HttpConnectionPool::doGet(const std::string &url, uint64_t timeout_ms,
        const std::map<std::string, std::string> &headers, const std::string &body) {
    return doRequest(HttpMethod::GET, url, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnectionPool::doPost(const std::string &url, uint64_t timeout_ms,
        const std::map<std::string, std::string> &headers, const std::string &body) {
    return doRequest(HttpMethod::POST, url, timeout_ms, headers, body);
}

HttpResult::ptr HttpConnectionPool::doGet(Uri::ptr uri, uint64_t timeout_ms,
        const std::map<std::string, std::string> &headers, const std::string &body) {
    std::stringstream ss;
    ss << uri->getPath()
        << (uri->getQuery().empty() ? "" : "?")
        << uri->getQuery()
        << (uri->getFragment().empty() ? "" : "#")
        << uri->getFragment();
    return doGet(ss.str(), timeout_ms, headers, body);
}

HttpResult::ptr HttpConnectionPool::doPost(Uri::ptr uri, uint64_t timeout_ms,
        const std::map<std::string, std::string> &headers, const std::string &body) {
    std::stringstream ss;
    ss << uri->getPath()
        << (uri->getQuery().empty() ? "" : "?")
        << uri->getQuery()
        << (uri->getFragment().empty() ? "" : "#")
        << uri->getFragment();
    return doPost(ss.str(), timeout_ms, headers, body);
}

HttpResult::ptr HttpConnectionPool::doRequest(HttpMethod method, const std::string &url, uint64_t timeout_ms,
        const std::map<std::string, std::string> &headers, const std::string &body) {
    HttpRequest::ptr req = std::make_shared<HttpRequest>();
    req->setMethod(method);
    req->setPath(url); // query fragment 在 url 中
    bool has_host{ false };
    for (auto& i : headers) {
        if (strcasecmp(i.first.c_str(), "connection") == 0) {
            if (strcasecmp(i.second.c_str(), "close") == 0) {
                req->setClose(false);
            }
        }
        if (strcasecmp(i.first.c_str(), "host") == 0) {
            has_host = !i.second.empty();
        }
        req->setHeader(i.first, i.second);
    }
    if (!has_host) {
        if (m_vhost.empty()) {
            req->setHeader("Host", m_host);
        } else {
            req->setHeader("Host", m_vhost);
        }
    }
    req->setBody(body);
    return doRequest(req, timeout_ms);
}

HttpResult::ptr HttpConnectionPool::doRequest(HttpMethod method, Uri::ptr uri, uint64_t timeout_ms,
        const std::map<std::string, std::string> &headers, const std::string &body) {
    std::stringstream ss;
    ss << uri->getPath()
        << (uri->getQuery().empty() ? "" : "?")
        << uri->getQuery()
        << (uri->getFragment().empty() ? "" : "#")
        << uri->getFragment();
    return doRequest(method, ss.str(), timeout_ms, headers, body);
}

HttpResult::ptr HttpConnectionPool::doRequest(HttpRequest::ptr req, uint64_t timeout_ms) {
    auto conn = getConnection();
    if (!conn) {
        return std::make_shared<HttpResult>(HttpResult::POOL_GET_CONNECTION_FAIL
            ,nullptr, "pool host:" + m_host + " port:" + std::to_string(m_port));
    }
    auto sock = conn->getSocket();
    if (!sock) {
        return std::make_shared<HttpResult>(HttpResult::POOL_INVALID_CONNECTION
            ,nullptr, "pool host:" + m_host + " port:" + std::to_string(m_port));
    }
    sock->setRecvTimeout(timeout_ms);
    int rt = conn->sendRequest(req);
    if (rt == 0) {
        return std::make_shared<HttpResult>(HttpResult::SEND_CLOSE_BY_PEER,
            nullptr, "send request closed by peer" + sock->getRemoteAddress()->toString());
    }
    if (rt < 0) {
        return std::make_shared<HttpResult>(HttpResult::SEND_SOCKET_ERROR,
            nullptr, "send request socket error errno=" + std::to_string(errno)
                + " errstr=" + strerror(errno));
    }
    HttpResponse::ptr rsp = conn->recvResponse();
    if (!rsp) {
        return std::make_shared<HttpResult>(HttpResult::TIMEOUT,
            nullptr, "recv response timeout: " + sock->getRemoteAddress()->toString() +
                + " timeout_ms:" + std::to_string(timeout_ms));
    }
    return std::make_shared<HttpResult>(HttpResult::OK, rsp, "ok");
}

void HttpConnectionPool::ReleasePtr(HttpConnection *ptr, HttpConnectionPool *pool) {
    if (!ptr->isConnected()
            || ptr->m_createTime + pool->m_maxAliveTime >= GetCurrentMS()) {
        delete ptr;
        --pool->m_total;
        return;
    }
    MutexType::Lock lock(pool->m_mutex);
    pool->m_conns.push_back(ptr);
}
}
