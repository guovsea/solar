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
                        return nullptr;
                    }
                    len += rt;
                }
                data[len] = '\0';
                size_t nparse = parser->execute(data, len, true);
                if (parser->hasError()) {
                    return nullptr;
                }
                len -= nparse;
                if (len == buff_size) {
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
        }
    }
    parser->getData()->setBody(body);
    return parser->getData();
}

int HttpConnection::sendRequest(HttpRequest::ptr req) {
    std::string data = req->toString();
    return writeFixSize(data.c_str(), data.size());
}
}
