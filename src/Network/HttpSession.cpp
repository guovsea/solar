//
// Created by guo on 2025/8/10.
//

#include "Network/HttpSession.h"
#include "Network/http_parser.h"

namespace solar::http {

HttpSession::HttpSession(Socket::ptr sock, bool owner)
    :SocketStream{sock, owner} {
}

HttpRequest::ptr HttpSession::recvRequest() {
    uint64_t buff_size = HttpRequestParser::GetHttpRequestBufferSize();
    std::vector<char> buffer(buff_size);
    char* data = buffer.data();
    HttpRequestParser::ptr parser = std::make_shared<HttpRequestParser>();
    int offset = 0;
    while (true) {
        int len = read(data + offset, buff_size - offset);
        if (len <= 0) {
            return nullptr;
        }
        len += offset;
        std::string tmp(data);
        int n = tmp.size();
        size_t nparse = parser->execute(data, len);
        if (parser->hasError()) {
            return nullptr;
        }
        offset = len - nparse;
        if (offset == buff_size) {
            return nullptr;
        }
        if (parser->isFinished()) {
            break;
        }
    }
    int64_t length = parser->getContextLength();
    if (length > 0) {
        std::string body;
        body.resize(length);
        if (length >= offset) {
            body.append(data, offset);
        } else {
            body.append(data, length);
        }
        body.append(data, offset);
        length -= offset;
        if (length > 0) {
          if (readFixSize(&body[body.size()], length) <=0) {
             return nullptr;
          }
            parser->getData()->setBody(body);
        }
    }
    return parser->getData();
}

int HttpSession::sendResponse(HttpResponse::ptr rsp) {
    std::string data = rsp->toString();
    return writeFixSize(data.c_str(), data.size());
}
}
