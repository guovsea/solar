//
// Created by guo on 2025/8/10.
//

#ifndef __SOLAR_SOCKETSTREAM_H__
#define __SOLAR_SOCKETSTREAM_H__

#include "Network/Stream.h"
#include "Network/Socket.h"

namespace  solar {
class SocketStream : public Stream {
public:
    typedef std::shared_ptr<SocketStream> ptr;

    SocketStream(Socket::ptr sock, bool owner = true);
    ~SocketStream() override;
    virtual int read(void *buffer, size_t length) override;
    virtual int read(ByteArray::ptr ba, size_t length) override;

    virtual int write(const void *buffer, size_t length) override;
    virtual int write(ByteArray::ptr ba, size_t length) override;
    virtual void close() override;

    Socket::ptr getSocket() const { return m_socket; };
    bool isConnected() const;
protected:
    Socket::ptr m_socket;
    bool m_owner;
};
}

#endif //__SOLAR_SOCKETSTREAM_H__