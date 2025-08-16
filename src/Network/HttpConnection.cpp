//
// Created by guo on 2025/8/13.
//

#include "HttpConnection.h"

#include "http_parser.h"

namespace solar::http {
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
            return nullptr;
        }
        len += offset;  // 读了之后，所有未解析的数据的长度
        data[len] = '\0'; // 避免 httpclient_parser.rl:197 的断言
        size_t nparse = parser->execute(data, len);
        if (parser->hasError()) {
            return nullptr;
        }
        offset = len - nparse; // 解析之后剩余未解析数据的长度
        if (offset == buff_size) {
            // 有效数据满了，buff 中所有数据都未被解析，因此不能再接收数据
            return nullptr;
        }
        if (parser->isFinished()) {
            break;
        }
    }
    // header 解析完毕, body 将 从 data 的起始位置开始
    // offset 为未解析的数据 (body) 的长度
    int64_t length = parser->getContextLength(); //  header 中的 "Content-Length"
    if (length > 0) {
        std::string body;
        body.resize(length);

        int len = 0;
        if (length <= offset) {
            memcpy(&body[0], data, length);
        } else {
            memcpy(&body[0], data, offset);
            length -= offset;
            // 根据 Content-Length，还有 length 长度的数据未到达，继续读取
            if (readFixSize(&body[body.size()], length) <=0) {
                return nullptr;
            }
        }
        parser->getData()->setBody(body);
    }
    return parser->getData();
}

int HttpConnection::sendRequest(HttpRequest::ptr req) {
    std::string data = req->toString();
    return writeFixSize(data.c_str(), data.size());
}
}
