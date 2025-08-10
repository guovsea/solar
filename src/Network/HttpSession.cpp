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
    int parse_left = 0; // buffer 中未被解析的字节数
    while (true) {
        int len = read(data + parse_left, buff_size - parse_left);
        if (len <= 0) {
            return nullptr;
        }
        len += parse_left;
        size_t nparse = parser->execute(data, len);
        if (parser->hasError()) {
            return nullptr;
        }
        parse_left = len - nparse;
        if (parse_left > 0) {
            // 将未解析数据移动到缓冲区起始位置
            memmove(data, data + nparse, parse_left);
        }
        if (parse_left == buff_size) {
            // 如果解析后 buffer 中还剩下的数据量 == 整个 buffer 的大小，就直接认为请求无效，返回 nullptr
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
        if (length >= parse_left) {
            body.append(data, parse_left);
        } else {
            body.append(data, length);
        }
        body.append(data, parse_left);
        length -= parse_left;
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
